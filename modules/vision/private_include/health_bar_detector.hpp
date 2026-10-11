#ifndef VISION_HEALTH_BAR_DETECTOR_HPP
#define VISION_HEALTH_BAR_DETECTOR_HPP

#include <opencv2/core.hpp>

namespace autoshinyhunthgss{
namespace vision {

// Returns the longest horizontal run of set pixels in any single row of a CV_8U mask.
// Gaps of up to gap_tolerance unset pixels are bridge and count toward the run's length.
int longest_run(const cv::Mat& mask, int gap_tolerance);

} // namespace vision
} // namespace autoshinyhunthgs

#endif // VISION_HEALTH_BAR_DETECTOR_HPP