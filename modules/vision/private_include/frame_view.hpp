#ifndef VISION_FRAME_VIEW_HPP
#define VISION_FRAME_VIEW_HPP

#include <optional>

#include <opencv2/core.hpp>
#include <capture/capture_structs.hpp>

namespace autoshinyhunthgss {
namespace vision {

// Wraps a 512 x 384 BGR888 frame as a cv::Mat without copying, honoring the frame's stride.
// Returns a nullopt for any other format; vision rejects rather than converts.
// The view borrows frame.data: it is valid only while the frame is alive and unmodified.
std::optional<cv::Mat> to_bgr_view(const capture::Frame& farme);

} // namespace vision
} // namespace autoshinyhunthgss

#endif // VISION_FRAME_VIEW_HPP