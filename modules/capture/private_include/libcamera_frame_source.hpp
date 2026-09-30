#ifndef CAPTURE_LIBCAMERA_FRAME_SOURCE_HPP
#define CAPTURE_LIBCAMERA_FRAME_SOURCE_HPP

#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>

#include <libcamera/libcamera.h>

#include <capture/frame_source_factory.hpp>
#include <capture/i_frame_source.hpp>

#include "frame_recorder.hpp"

namespace autoshinyhunthgss {
namespace capture {

/*
Captures from the Pi camera through libcamera, one frame per capture_frame() call.

A single request is queued only when a frame is asked for, so the buffer holds a frame
exposed after the call started, never one left over from before the hunt loop slept.
Nothing is in flight while the caller works, so the mapped buffer is read without copying.

The cost is one to two frame periods of waiting per call, which suits a loop that polls.
A loop needing every frame would need a pool of requests instead.
*/
class LibcameraFrameSource : public IFrameSource
{
public:
    explicit LibcameraFrameSource(const CameraSourceConfig& config);
    ~LibcameraFrameSource() override;

    LibcameraFrameSource(const LibcameraFrameSource&) = delete;
    LibcameraFrameSource& operator=(const LibcameraFrameSource&) = delete;

    // Acquires and starts the camera. Returns false, having logged why, if it cannot
    bool init();

    // Captures a photo and locates the screen
    CaptureStatus capture_frame(Frame& out_frame) override;

private:
    // Validate the image format with the camera
    bool configure_stream();

    // Sets up the memory the camera writes into.
    // libcamera allocates a DMA buffer, which the kernel shares with the ISP hardware.
    // mmap then makes those same bytes readable for processing.
    // Nothing gets copied: the camera writes into the buffer and is read.
    //
    // sensor (via ISP) --writes--> DMA buffer <--reads-- _pixels (via mmap)
    //
    bool map_buffer();

    // Callback method. Libcamera calls it on its thread when a request finishes.
    // It simply sets the flag and wakes the hunt thread. All heavy work stays on the hunt thread.
    void on_request_completed(libcamera::Request* request); // runs on libcamera's thread

    // Undoes init() in reverse order
    void teardown();

    CameraSensorResolution _resolution;
    bool _is_every_frame_retained;
    FrameRecorder _frame_recorder;

    std::unique_ptr<libcamera::CameraManager> _camera_manager;
    std::shared_ptr<libcamera::Camera> _camera;
    std::unique_ptr<libcamera::CameraConfiguration> _camera_config;
    std::unique_ptr<libcamera::FrameBufferAllocator> _allocator;
    std::unique_ptr<libcamera::Request> _request;
    libcamera::Stream* _stream = nullptr; // handle to the camera's output channel
    libcamera::FrameBuffer* _buffer = nullptr; // memory the camera writes pixels into

    void *_mapped = nullptr;
    std::size_t _mapped_length = 0;
    const std::uint8_t *_pixels = nullptr;
    int _stride = 0;

    bool _is_manager_started = false;
    bool _is_acquired = false;
    bool _is_started = false;
    bool _is_request_in_flight = false; // hunt thread only

    // Two threads use shared variable _is_request_done
    // 1. The hunt thread resets it before queueing, then waits on it.
    // 2. The libcamera thread writes it when the request completes and the frame is in the buffer.
    //
    // The condition variable lets the hunt thread sleep until libcamera wakes it,
    // instead of spinning in a loop checking the flag.
    std::mutex _mutex;
    std::condition_variable _request_done;
    bool _is_request_done = false; // guarded by _mutex
};

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_LIBCAMERA_FRAME_SOURCE_HPP