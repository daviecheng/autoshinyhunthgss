#include "libcamera_frame_source.hpp"

#include <chrono>
#include <iostream>
#include <string>

#include <sys/mman.h>

#include "frame_conversion.hpp"
#include "screen_locator.hpp"

namespace autoshinyhunthgss {
namespace capture {

namespace
{

// The resolution the locator's ~23ms/frame budget was measured at.
// The full sensor would cost seconds per frame, and the ISP scales down for free.
constexpr CameraSensorResolution kDefaultResolution{1280, 960};

// Long enough to ride out auto-exposure settling on the first frame, short enough
// that a dead camera surfaces as Unknown rather than a hang.
constexpr std::chrono::milliseconds kRequestTimeout{1000};

} // namespace

LibcameraFrameSource::LibcameraFrameSource(const CameraSourceConfig& config)
    : _resolution(config.resolution.value_or(kDefaultResolution))
    , _is_every_frame_retained(config.retention.is_every_frame_retained)
    , _frame_recorder(config.retention)
{}

LibcameraFrameSource::~LibcameraFrameSource()
{
    teardown();
}

bool LibcameraFrameSource::init()
{
    // This method does the following:
    // 1. Start manager, pick camera 0, acquire it
    // 2. Validate image format with camera
    // 3. Set up the memory the camera writes into
    // 4. Create 1 request and attach the buffer
    // 5. Connect callback method to libcamera's signal
    // 6. Start camera

    // Start manager, pick camera 0, acquire it
    _camera_manager = std::make_unique<libcamera::CameraManager>();
    if (_camera_manager->start() != 0)
    {
        std::cerr << "capture: Cannot start the libcamera camera manager\n";
        return false;
    }
    _is_manager_started = true;

    const auto cameras = _camera_manager->cameras();
    if (cameras.empty())
    {
        std::cerr << "capture: No cameras found\n";
        return false;
    }
    _camera = cameras.front();

    if (_camera->acquire() != 0)
    {
        std::cerr << "capture: Cannot acquire camera " << _camera->id()
                  << " (is rpicam-* or another run still holding it?)\n";
        return false;
    }
    _is_acquired = true;

    if (!configure_stream() || !map_buffer())
    {
        return false;
    }

    _request = _camera->createRequest();
    if (!_request || _request->addBuffer(_stream, _buffer) != 0)
    {
        std::cerr << "capture: Cannot create a capture request\n";
        return false;
    }

    _camera->requestCompleted.connect(this, &LibcameraFrameSource::on_request_completed);

    if (_camera->start() != 0)
    {
        std::cerr << "capture: Cannot start camera " << _camera->id() << "\n";
        return false;
    }
    _is_started = true;

    std::cout << "capture: Streaming " << _camera_config->at(0).toString()
              << " from " << _camera->id() << "\n";

    return true;
}

CaptureStatus LibcameraFrameSource::capture_frame(Frame& out_frame)
{
    // This method does the following:
    // 1. Check whether a request is already out.
    // 2. Clear the old request data, but reuse memory buffer attached
    // 3. Mark the request "not done yet" before sending so the camera's "done" signal can mark it to "done".
    // 4. Hand the request to the camera. queueRequest() asks libcamera to fill the buffer with the next photo.
    //      - queue fails: return CaptureStatus::Unknown; the next call tries again
    //      - queue succeeds: mark the request in flight so no later call sends it twice
    // 5. Sleep until libcamera's thread marks the request done, or give up after kRequestTimeout.

    if (!_is_request_in_flight)
    {
        _request->reuse(libcamera::Request::ReuseBuffers);

        {
            // There should be no contention since no request is in flight
            std::lock_guard<std::mutex> lock(_mutex);
            _is_request_done = false;
        }

        if (_camera->queueRequest(_request.get()) != 0)
        {
            std::cerr << "capture: Cannot queue a capture request\n";
            return CaptureStatus::Unknown;
        }

        _is_request_in_flight = true;
    }

    {
        std::unique_lock<std::mutex> lock(_mutex);
        if (!_request_done.wait_for(lock, kRequestTimeout, [this] { return _is_request_done; }))
        {
            std::cerr << "capture: Timed out waiting for a frame\n";
            return CaptureStatus::Unknown;
        }
    }

    _is_request_in_flight = false;

    const libcamera::FrameMetadata& metadata = _buffer->metadata();

    if ( _request->status() != libcamera::Request::RequestComplete
         || metadata.status != libcamera::FrameMetadata::FrameSuccess)
    {
        std::cerr << "capture: Frame rejected, request status " << _request->status()
                  << ", frame status " << metadata.status << "\n";
        return CaptureStatus::Unknown;
    }

    // Wraps the mapped buffer without copying. It stays valid until the request is
    // queued again, which only happens at the start of the next call.
    const libcamera::Size& size = _camera_config->at(0).size;
    const cv::Mat input_frame(static_cast<int>(size.height),
                              static_cast<int>(size.width),
                              CV_8UC3,
                              const_cast<std::uint8_t*>(_pixels),
                              static_cast<std::size_t>(_stride));

    cv::Mat output_screen;
    const CaptureStatus status = screen_locator::locate(input_frame, output_screen);

    if (status != CaptureStatus::ScreenFound)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(status)));
        return status;
    }

    const FrameConversionStatus conversion = fill_frame(output_screen,
                                                        static_cast<std::int64_t>(metadata.timestamp),
                                                        metadata.sequence,
                                                        out_frame);

    if (conversion != FrameConversionStatus::Success)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(conversion)));
        return CaptureStatus::Unknown;
    }

    if (_is_every_frame_retained)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(status)));
    }

    return CaptureStatus::ScreenFound;
}


bool LibcameraFrameSource::configure_stream()
{
    _camera_config = _camera->generateConfiguration({libcamera::StreamRole::Viewfinder});
    if (!_camera_config)
    {
        std::cerr << "capture: Camera offers no viewfinder configuration\n";
        return false;
    }

    libcamera::StreamConfiguration& stream_config = _camera_config->at(0);

    // libcamera follows DRM naming, which lists channels from the most significant byte
    // down, so RGB888 should land in memory as B, G, R.
    stream_config.pixelFormat = libcamera::formats::RGB888;
    stream_config.size = libcamera::Size(_resolution.width, _resolution.height);
    stream_config.bufferCount = 1;

    const std::string requested = stream_config.toString();

    switch (_camera_config->validate())
    {
        case libcamera::CameraConfiguration::Invalid:
            std::cerr << "capture: Camera rejected " << requested << "\n";
            return false;
        case libcamera::CameraConfiguration::Adjusted:
            std::cout << "capture: requested " << requested
                      << ", camera adjusted to "  << stream_config.toString() << "\n";
            break;
        case libcamera::CameraConfiguration::Valid:
            break;
    }

    // The cv::Mat wrap assumes 3 packed bytes per pixel; any other format would be
    // misread silently rather than fail.
    if (stream_config.pixelFormat != libcamera::formats::RGB888)
    {
        std::cerr << "capture: Camera cannot produce RGB888, offered "
                  << stream_config.pixelFormat.toString() << "\n";
        return false;
    }

    if (_camera->configure(_camera_config.get()) != 0)
    {
        std::cerr << "capture: Cannot configure camera\n";
        return false;
    }

    _stream = stream_config.stream();
    _stride = static_cast<int>(stream_config.stride); // ISP pads rows; never assume width * 3

    return true;
}

bool LibcameraFrameSource::map_buffer()
{
    _allocator = std::make_unique<libcamera::FrameBufferAllocator>(_camera);
    if (_allocator->allocate(_stream) < 1)
    {
        std::cerr << "capture: Cannot allocate frame buffers\n";
        return false;
    }

    // Validation may raise bufferCount to the pipeline's minimum; one is all this needs.
    _buffer = _allocator->buffers(_stream).front().get();

    // RGB888 is packed, so the whole image is one plane.
    const libcamera::FrameBuffer::Plane& plane = _buffer->planes().front();

    // mmap offsets must be page aligned and plane.offset need not be, so map the dmabuf
    // from its start and step to the plane afterwards.
    _mapped_length = plane.offset + plane.length;
    _mapped = mmap(nullptr, _mapped_length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);
    if (_mapped == MAP_FAILED)
    {
        _mapped = nullptr;
        std::cerr << "capture: Cannot map the frame buffer\n";
        return false;
    }

    _pixels = static_cast<const std::uint8_t*>(_mapped) + plane.offset;

    return true;
}

void LibcameraFrameSource::on_request_completed(libcamera::Request *request)
{
    if (request != _request.get())
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        _is_request_done = true;
    }

    _request_done.notify_one();
}

void LibcameraFrameSource::teardown()
{
    // Stops the camera before unmapping, so nothing writes into memory that has been released
    // Release the camera before stopping the manager, otherwise libcamera aborts.

    if (_is_started)
    {
        // Cancels any request still in flight, so nothing writes the buffer after this
        _camera->stop();
        _is_started = false;
    }

    if (_camera)
    {
        _camera->requestCompleted.disconnect(this);
    }

    _request.reset();

    if (_mapped)
    {
        munmap(_mapped, _mapped_length);
        _mapped = nullptr;
    }

    if (_allocator && _stream)
    {
        _allocator->free(_stream);
    }
    _allocator.reset();

    if (_is_acquired)
    {
        _camera->release();
        _is_acquired = false;
    }

    _camera_config.reset();

    // The camera must be released before the manager stops, or libcamera aborts on exit.
    _camera.reset();

    if (_is_manager_started)
    {
        _camera_manager->stop();
        _is_manager_started = false;
    }
    _camera_manager.reset();
}

} // namespace capture
} // namespace autoshinyhunthgss