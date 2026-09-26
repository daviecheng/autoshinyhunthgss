#ifndef CAPTURE_IMAGE_FRAME_SOURCE_HPP
#define CAPTURE_IMAGE_FRAME_SOURCE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <capture/frame_source_factory.hpp>
#include <capture/i_frame_source.hpp>

#include "frame_recorder.hpp"

namespace autoshinyhunthgss {
namespace capture {

class ImageFrameSource : public IFrameSource
{
public:
    explicit ImageFrameSource(const ImageSourceConfig& config);

    CaptureStatus capture_frame(Frame& out_frame) override;

private:
    std::vector<std::string> _image_paths;
    bool _is_every_frame_retained;
    std::size_t _next_index = 0;
    FrameRecorder _frame_recorder;
};

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_IMAGE_FRAME_SOURCE_HPP
