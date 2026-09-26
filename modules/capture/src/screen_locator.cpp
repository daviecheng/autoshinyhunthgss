#include "screen_locator.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>
#include <capture/capture_constants.hpp>

#include "opencv_extensions.hpp"
#include "capture_constants_p.hpp"

namespace autoshinyhunthgss {
namespace capture {
namespace screen_locator {

CaptureStatus locate(const cv::Mat& input_frame, cv::Mat& out_screen)
{
    if (input_frame.empty() || input_frame.type() != CV_8UC3)
    {
        return CaptureStatus::ScreenNotFound;
    }

    std::vector<cv::Point2f> corners;
    if (!find_screen_corners(input_frame, corners))
    {
        return CaptureStatus::ScreenNotFound;
    }

    out_screen = opencv_extensions::warp_to_size(input_frame, corners, cv::Size(kFrameWidth, kFrameHeight));

    return CaptureStatus::ScreenFound;
}

bool find_screen_corners(const cv::Mat& input_frame, std::vector<cv::Point2f>& out_corners)
{
    const auto contours = opencv_extensions::find_edge_contours(
                                                input_frame,
                                                kBlurKernelSize,
                                                kCannyLowThreshold,
                                                kCannyHighThreshold,
                                                kMorphologyKernelSize);

    const double minimum_area = kMinScreenAreaRatio * static_cast<double>(input_frame.total());
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

} // namespace screen_locator
} // namespace capture
} // namespace autoshinyhunthgss
