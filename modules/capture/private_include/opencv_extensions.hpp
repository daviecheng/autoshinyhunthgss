#ifndef CAPTURE_OPENCV_EXTENSIONS_HPP
#define CAPTURE_OPENCV_EXTENSIONS_HPP

#include <vector>

#include <opencv2/core.hpp>

namespace autoshinyhunthgss {
namespace capture {
namespace opencv_extensions {

// Reorders an arbitrary quadrilateral top-left, top-right, bottom-right, bottom-left
std::vector<cv::Point2f> order_corners(const std::vector<cv::Point>& quad);

// Mean width over mean height, compared against an expected ratio
bool has_plausible_aspect_ratio(const std::vector<cv::Point2f>& ordered_corners,
                                double expected_aspect_ratio,
                                double tolerance);

// Outer contours of the bright regions of a BGR image, split by Otsu
// Otsu picks one brightess cutoff and splits the image into light and dark images
// Otsu finds areas.
// Doesn't work well with current setup, but this may be useful later.
std::vector<std::vector<cv::Point>> find_bright_contours(const cv::Mat& bgr_image);

// Contours of the edges in a BGR image.
// Canny ignores absolute brightness and marks pixels where brightness changes sharply.
// Canny finds outlines.
std::vector<std::vector<cv::Point>> find_edge_contours(
                                        const cv::Mat& bgr_image,
                                        int blur_kernel_size,
                                        double canny_low_threshold,
                                        double canny_high_threshold,
                                        int close_kernel_size);

// Douglas-Peucker simplification, epsilon given as a fraction of perimeter
std::vector<cv::Point> approximate_polygon(const std::vector<cv::Point>& contour, double epsilon_ratio);

// Rectifies the quadrilateral described by ordered_corners into a flat image
cv::Mat warp_to_size(const cv::Mat& source, const std::vector<cv::Point2f>& ordered_corners, const cv::Size& output_size);

} // namespace opencv_extensions
} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_OPENCV_EXTENSIONS_HPP
