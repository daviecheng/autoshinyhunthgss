#include "band_mask.hpp"

#include <opencv2/core.hpp>

namespace autoshinyhunthgss {
namespace vision {

namespace {

constexpr int kMaxHue = 179; // OpenCV 8-bit hue is 0-179
constexpr int kMaxChannel = 255; // S and V have no upper limit

} // namespace

cv::Mat band_mask(const cv::Mat& hsv, const HsvBand& band)
{
    cv::Mat mask;

    if (band.h_lo <= band.h_hi)
    {
        cv::inRange(hsv,
                    cv::Scalar(band.h_lo, band.s_min, band.v_min),
                    cv::Scalar(band.h_hi, kMaxChannel, kMaxChannel),
                    mask);

        return mask;
    }

    // Wrapping band: union of [h_lo, 179] and [0, h_hi]
    cv::Mat upper;
    cv::Mat lower;

    cv::inRange(hsv,
                cv::Scalar(band.h_lo, band.s_min, band.v_min),
                cv::Scalar(kMaxHue, kMaxChannel, kMaxChannel),
                upper);

    cv::inRange(hsv,
                cv::Scalar(0, band.s_min, band.v_min),
                cv::Scalar(band.h_hi, kMaxChannel, kMaxChannel),
                lower);

    cv::bitwise_or(upper, lower, mask); // combine the two stencils

    return mask;
}

} // namespace vision
} // namespace autoshinyhunthg