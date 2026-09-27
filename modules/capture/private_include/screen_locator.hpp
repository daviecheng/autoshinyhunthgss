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

// Removes speckle from a binary mask, then closes gaps inside its regions.
void clean_mask(cv::Mat& mask);

// Keeps whichever region in the mask is closes to the screen's aspect ratio,
// replacing out_corners and out_best_score when it beats them.
void collect_screen_candidates(const cv::Mat& mask,
                               const cv::Mat& input_frame,
                               std::vector<cv::Point2f>& out_corners,
                               double& out_best_score);

} // namespace screen_locator
} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_SCREEN_LOCATOR_HPP