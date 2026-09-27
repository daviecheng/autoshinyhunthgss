#include "screen_locator.hpp"

#include <cmath>
#include <limits>

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
    cv::Mat grayscale;
    cv::cvtColor(input_frame, grayscale, cv::COLOR_BGR2GRAY);

    cv::Mat hsv;
    cv::cvtColor(input_frame, hsv, cv::COLOR_BGR2HSV);

    std::vector<cv::Mat> hsv_channels;
    cv::split(hsv, hsv_channels);

    double best_score = std::numeric_limits<double>::max();
    std::vector<cv::Point2f> best_corners;
    cv::Mat mask;

    for (const int threshold : kDarkThresholds)
    {
        cv::threshold(grayscale, mask, threshold, 255, cv::THRESH_BINARY_INV);
        clean_mask(mask);
        collect_screen_candidates(mask, input_frame, best_corners, best_score);
    }

    for (const int threshold : kBrightThresholds)
    {
        cv::threshold(grayscale, mask, threshold, 255, cv::THRESH_BINARY);
        clean_mask(mask);
        collect_screen_candidates(mask, input_frame, best_corners, best_score);
    }

    for (const int threshold : kSaturationThresholds)
    {
        cv::threshold(hsv_channels[1], mask, threshold, 255, cv::THRESH_BINARY);
        clean_mask(mask);
        collect_screen_candidates(mask, input_frame, best_corners, best_score);
    }

    if (best_corners.empty())
    {
        return false;
    }

    out_corners = best_corners;

    return true;
}

void clean_mask(cv::Mat& mask)
{
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(kMaskKernelSize, kMaskKernelSize));

    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    cv::morphologyEx(mask, mask, cv::MORPH_CLOSE, kernel);
}

void collect_screen_candidates(const cv::Mat& mask,
                               const cv::Mat& input_frame,
                               std::vector<cv::Point2f>& out_corners,
                               double& out_best_score)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    for (const auto& contour : contours)
    {
        const double area_ratio = cv::contourArea(contour) / static_cast<double>(input_frame.total());
        if (area_ratio < kMinScreenAreaRatio || area_ratio > kMaxScreenAreaRatio)
        {
            continue;
        }

        // The hull discards the ragged interior detail a raw contour carries, so
        // the screen border can simplify to four corners.
        std::vector<cv::Point> hull;
        cv::convexHull(contour, hull);

        for (double epsilon = kPolygonEpsilonMin; epsilon <= kPolygonEpsilonMax; epsilon += kPolygonEpsilonStep)
        {
            const auto polygon = opencv_extensions::approximate_polygon(hull, epsilon);
            if (polygon.size() != 4)
            {
                continue;
            }

            if (!cv::isContourConvex(polygon))
            {
                break;
            }

            const auto ordered = opencv_extensions::order_corners(polygon);
            if (!opencv_extensions::has_plausible_aspect_ratio(ordered, kExpectedAspectRatio, kAspectRatioTolerance))
            {
                break;
            }

            // Largest-first is wrong here: a lit window outvotes the screen.
            // The screen is whichever region is shaped most like a DS screen.
            const double score = std::abs(opencv_extensions::aspect_ratio(ordered) - kExpectedAspectRatio);
            if (score < out_best_score)
            {
                out_corners = ordered;
                out_best_score = score;
            }

            break;
        }
    }
}

} // namespace screen_locator
} // namespace capture
} // namespace autoshinyhunthgss
