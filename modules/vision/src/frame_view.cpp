#include "frame_view.hpp"

#include <cstddef>
#include <cstdint>

namespace autoshinyhunthgss {
namespace vision {

std::optional<cv::Mat> to_bgr_view(const capture::Frame& frame)
{
    if (!capture::has_expected_format(frame))
    {
        return std::nullopt;
    }

    // cv::Mat has no read-only variant, so the const has to go. Vision only ever reads the view.
    auto* data = const_cast<std::uint8_t*>(frame.data.data());

    return cv::Mat(frame.height, frame.width, CV_8UC3, data, static_cast<std::size_t>(frame.stride));
}

} // namespace vision
} // namespace autoshinyhunthgss