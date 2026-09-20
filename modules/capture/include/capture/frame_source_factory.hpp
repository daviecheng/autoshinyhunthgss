#ifndef CAPTURE_FRAME_SOURCE_FACTORY_HPP
#define CAPTURE_FRAME_SOURCE_FACTORY_HPP

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <capture/i_frame_source.hpp>

namespace autoshinyhunthgss {
namespace capture {

struct CameraSensorResolution
{
    int width = 0;
    int height = 0;
};

struct FrameRetentionConfig
{
    std::string output_directory; // disables retention if empty
    int max_retained_frames = 50;
    bool is_input_frame_saved = true;
    bool is_output_frame_saved = true;
};

struct ImageSourceConfig
{
    std::vector<std::string> image_paths;
    bool is_looping = true;
    FrameRetentionConfig retention;
};

struct CameraSourceConfig
{
    std::optional<CameraSensorResolution> resolution;
    FrameRetentionConfig retention;
};

// Replays stored photos. Returns nullptr when image_paths is empty
std::unique_ptr<IFrameSource> make_image_frame_source(const ImageSourceConfig& config);

// Streams from the Pi camera. Returns nullptr when built without libcamera support, or when no camera can be acquired
std::unique_ptr<IFrameSource> make_camera_frame_source(const CameraSourceConfig& config);

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_FRAME_SOURCE_FACTORY_HPP
