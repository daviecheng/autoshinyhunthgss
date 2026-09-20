#include "screen_locator.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>
#include <capture/capture_constants.hpp>

#include "opencv_extensions.hpp"
#include "capture_constants_p.hpp"

namespace autoshinyhunthgss {
namespace capture {

CaptureStatus ScreenLocator::locate(const cv::Mat& wide_frame, cv::Mat& out_screen)
{
    if (wide_frame.empty() || wide_frame.type() != CV_8UC3)
    {
        return CaptureStatus::ScreenNotFound;
    }

    std::vector<cv::Point2f> corners;

    if (find_screen_corners(wide_frame, corners))
    {
        _last_good_corners = corners;
    }
    else if (!_last_good_corners.empty())
    {
        corners = _last_good_corners;
    }
    else
    {
        return CaptureStatus::ScreenNotFound;
    }

    out_screen = opencv_extensions::warp_to_size(wide_frame, corners, cv::Size(kFrameWidth, kFrameHeight));

    return CaptureStatus::ScreenFound;
}

bool ScreenLocator::find_screen_corners(const cv::Mat& wide_frame, std::vector<cv::Point2f>& out_corners)
{
    const auto contours = opencv_extensions::find_edge_contours(
                                                wide_frame, 
                                                kBlurKernelSize, 
                                                kCannyLowThreshold, 
                                                kCannyHighThreshold, 
                                                kMorphologyKernelSize);

    const double minimum_area = kMinScreenAreaRatio * static_cast<double>(wide_frame.total());
    double best_area = 0.0;
    bool has_match = false;

    for (const auto& contour : contours)
    {
        const double area = cv::contourArea(contour);
        if (area < minimum_area || area <= best_area)
        {
            continue;
        }

        const auto polygon = opencv_extensions::approximate_polygon(contour, kPolygonEpsilonRatio);
        if (polygon.size() != 4 || !cv::isContourConvex(polygon))
        {
            continue;
        }

        const auto ordered = opencv_extensions::order_corners(polygon);
        if (!opencv_extensions::has_plausible_aspect_ratio(ordered, kExpectedAspectRatio, kAspectRatioTolerance))
        {
            continue;
        }

        out_corners = ordered;
        best_area = area;
        has_match = true;
    }

    return has_match;
}

} // namespace capture
} // namespace autoshinyhunthgss
