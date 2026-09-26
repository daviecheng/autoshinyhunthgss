#ifndef CAPTURE_SCREEN_LOCATOR_HPP
#define CAPTURE_SCREEN_LOCATOR_HPP

#include <vector>

#include <opencv2/core.hpp>

#include <capture/capture_enums.hpp>

namespace autoshinyhunthgss {
namespace capture {
namespace screen_locator {

// Finds the DS top screen in a camera frame and rectifies it to kFrameWidth x kFrameHeight
CaptureStatus locate(const cv::Mat& input_frame, cv::Mat& out_screen);

// Finds the screen's four corners, ordered to-left, top-right, bottom-right, bottom-left
bool find_screen_corners(const cv::Mat& input_frame, std::vector<cv::Point2f>& out_corners);

} // namespace screen_locator
} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_SCREEN_LOCATOR_HPP