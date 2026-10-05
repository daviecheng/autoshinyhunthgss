#ifndef VISION_BAND_MASK_HPP
#define VISION_BAND_MASK_HPP

#include <opencv2/core.hpp>
#include "vision_config.hpp"

namespace autoshinyhunthgss {
namespace vision {

// Returns a CV_8U mask (255 = in band) of the HSV pixels inside the band and above its S/V floors.
// A band with h_lo > h_hi wraps past 179, e.g. red{176, 7} covers 176-179 and 0-7.
cv::Mat band_mask(const cv::Mat& hsv, const HsvBand& band);

} // namespace vision
} // namespace autoshinyhunthgss

#endif // VISION_BAND_MASK_HPP