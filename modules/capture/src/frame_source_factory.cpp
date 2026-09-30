#include <capture/frame_source_factory.hpp>

#include <iostream>

#include "image_frame_source.hpp"

#ifdef CAPTURE_HAS_LIBCAMERA
    #include "libcamera_frame_source.hpp"
#endif

namespace autoshinyhunthgss {
namespace capture {

std::unique_ptr<IFrameSource> make_image_frame_source(const ImageSourceConfig& config)
{
    if (config.image_paths.empty())
    {
        std::cerr << "capture: No images given for an image frame source\n";
        return nullptr;
    }

    return std::make_unique<ImageFrameSource>(config);
}

std::unique_ptr<IFrameSource> make_camera_frame_source(const CameraSourceConfig& config)
{
#ifdef CAPTURE_HAS_LIBCAMERA
    auto source = std::make_unique<LibcameraFrameSource>(config);

    // A failed init has already logged why; the destructor releases whatever was acquired.
    if (!source->init())
    {
        return nullptr;
    }

    return source;
#else
    (void)config;
    std::cerr << "capture: Built without libcamera; install libcamera and reconfigure\n";
    return nullptr;
#endif
}

} // namespace capture
} // namespace autoshinyhunthgss
