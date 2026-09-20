#include "opencv_extensions.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace autoshinyhunthgss {
namespace capture {
namespace opencv_extensions {


std::vector<cv::Point2f> order_corners(const std::vector<cv::Point>& quad)
{
    const auto sum_less = [](const cv::Point& a, const cv::Point& b) {
        return (a.x + a.y) < (b.x + b.y);
    };
    const auto difference_less = [](const cv::Point& a, const cv::Point& b)
    {
        return (a.y - a.x) < (b.y - b.x);
    };

    const auto sum_bounds        = std::minmax_element(quad.begin(), quad.end(), sum_less);
    const auto difference_bounds = std::minmax_element(quad.begin(), quad.end(), difference_less);

    std::vector<cv::Point2f> ordered(4);
    ordered[0] = *sum_bounds.first;         // top-left 
    ordered[1] = *difference_bounds.first;  // top-right
    ordered[2] = *sum_bounds.second;        // bottom-right
    ordered[3] = *difference_bounds.second; // bottom-left

    return ordered;
}

bool has_plausible_aspect_ratio(const std::vector<cv::Point2f>& ordered_corners,
                                double expected_aspect_ratio,
                                double tolerance)
{
    if (ordered_corners.size() != 4)
    {
        return false;
    }

    const double top    = cv::norm(ordered_corners[1] - ordered_corners[0]);
    const double bottom = cv::norm(ordered_corners[2] - ordered_corners[3]);
    const double left   = cv::norm(ordered_corners[3] - ordered_corners[0]);
    const double right  = cv::norm(ordered_corners[2] - ordered_corners[1]);

    const double width = (top + bottom) / 2.0;
    const double height = (left + right) / 2.0;

    if (height <= 0.0)
    {
        return false;
    }

    return std::abs(width / height - expected_aspect_ratio) <= tolerance;
}
    

std::vector<std::vector<cv::Point>> find_bright_contours(const cv::Mat& bgr_image)
{
    cv::Mat grayscale;
    cv::cvtColor(bgr_image, grayscale, cv::COLOR_BGR2GRAY);

    cv::Mat binary;
    cv::threshold(grayscale, binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    return contours;
}

std::vector<std::vector<cv::Point>> find_edge_contours(
                                        const cv::Mat& bgr_image,
                                        int blur_kernel_size,
                                        double canny_low_threshold,
                                        double canny_high_threshold,
                                        int close_kernel_size)
{
    cv::Mat grayscale;
    cv::cvtColor(bgr_image, grayscale, cv::COLOR_BGR2GRAY);

    cv::Mat blurred;
    cv::GaussianBlur(grayscale, blurred, cv::Size(blur_kernel_size, blur_kernel_size), 0);

    cv::Mat edges;
    cv::Canny(blurred, edges, canny_low_threshold, canny_high_threshold);

    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(close_kernel_size, close_kernel_size));
    cv::morphologyEx(edges, edges, cv::MORPH_CLOSE, kernel);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

    return contours;
}

std::vector<cv::Point> approximate_polygon(const std::vector<cv::Point>& contour, double epsilon_ratio)
{
    std::vector<cv::Point> polygon;
    cv::approxPolyDP(contour, polygon, epsilon_ratio * cv::arcLength(contour, true), true);

    return polygon;
}

cv::Mat warp_to_size(const cv::Mat& source, const std::vector<cv::Point2f>& ordered_corners, const cv::Size& output_size)
{
    const std::vector<cv::Point2f> destination = {
        {0.0F, 0.0F},
        {static_cast<float>(output_size.width - 1), 0.0F},
        {static_cast<float>(output_size.width - 1), static_cast<float>(output_size.height - 1)},
        {0.0F, static_cast<float>(output_size.height - 1)}
    };

    const cv::Mat transform = cv::getPerspectiveTransform(ordered_corners, destination);

    cv::Mat warped;
    cv::warpPerspective(source, warped, transform, output_size);

    return warped;
}

} // namespace opencv_extensions
} // namespace capture
} // namespace autoshinyhunthgss