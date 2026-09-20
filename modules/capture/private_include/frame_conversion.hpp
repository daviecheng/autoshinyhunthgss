#ifndef CAPTURE_FRAME_CONVERSION_HPP
#define CAPTURE_FRAME_CONVERSION_HPP

#include <vector>
#include <opencv2/core.hpp>
#include <capture/capture_structs.hpp>
#include "capture_enums_p.hpp"

namespace autoshinyhunthgss {
namespace capture {

// Copies a 512x384 CV_8UC3 screen into the Frame data structure
FrameConversionStatus fill_frame(const cv::Mat& screen,
                                std::int64_t timestamp_ns,
                                std::uint32_t sequence,
                                Frame& out_frame);

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_FRAME_CONVERSION_HPP
