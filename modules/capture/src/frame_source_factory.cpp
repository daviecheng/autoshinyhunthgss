#include <capture/frame_source_factory.hpp>

#include <iostream>

#include "image_frame_source.hpp"

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
    // TODO
    return nullptr;
}

} // namespace capture
} // namespace autoshinyhunthgss
