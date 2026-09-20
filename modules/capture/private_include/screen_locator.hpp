#ifndef CAPTURE_SCREEN_LOCATOR_HPP
#define CAPTURE_SCREEN_LOCATOR_HPP

#include <vector>

#include <opencv2/core.hpp>

#include <capture/capture_enums.hpp>

namespace autoshinyhunthgss {
namespace capture {

class ScreenLocator
{
public:
    CaptureStatus locate(const cv::Mat& wide_frame, cv::Mat& out_screen);

private:
    std::vector<cv::Point2f> _last_good_corners;

    bool find_screen_corners(const cv::Mat& wide_frame, std::vector<cv::Point2f>& out_corners);
};
    
} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_SCREEN_LOCATOR_HPP