#include "screen_locator.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

#include <opencv2/imgproc.hpp>
#include <capture/capture_constants.hpp>

#include "opencv_extensions.hpp"
#include "capture_constants_p.hpp"

namespace autoshinyhunthgss {
namespace capture {
namespace screen_locator {

namespace {

// Which side of a line the screen lies on. Each line's normal points down (horizontal
// lines) or right (vertical lines), so a top or left side has the screen on the positive
// side, and a bottom or right side on the negative side.
enum ScreenSide { kPositiveSide = 0, kNegativeSide = 1 };

// Fractions of samples along a line showing each cue, kept as prefix sums so any span
// between two corners is scored in constant time.
struct SideEvidence
{
    std::vector<int> rim_prefix;
    std::vector<int> glow_prefix;
};

// A detected straight border, extended across the whole working image.
struct EdgeLine
{
    cv::Point2f origin;
    cv::Point2f direction;
    cv::Point2f normal;
    float length = 0.0F;

    // Samples are one working pixel apart, starting at parameter t_begin along direction.
    float t_begin = 0.0F;
    std::vector<int> valid_prefix;
    std::array<SideEvidence, 2> evidence;
};

float sample(const cv::Mat& grayscale, const cv::Point2f& point)
{
    const int x = cvRound(point.x);
    const int y = cvRound(point.y);

    if (x < 0 || y < 0 || x >= grayscale.cols || y >= grayscale.rows)
    {
        return -1.0F;
    }

    return grayscale.at<std::uint8_t>(y, x);
}

// Darkest or brightest of the samples at the given offsets along a direction, or -1 when
// none falls inside the image. Samples past the edge are skipped rather than voiding the
// rest, so a screen a few pixels from the frame edge is still judged.
template <std::size_t N>
float sample_extreme(const cv::Mat& grayscale,
                     const cv::Point2f& point,
                     const cv::Point2f& direction,
                     const float (&offsets)[N],
                     bool is_darkest)
{
    float extreme = -1.0F;

    for (const float offset : offsets)
    {
        const float value = sample(grayscale, point + direction * offset);
        if (value < 0.0F)
        {
            continue;
        }

        extreme = (extreme < 0.0F) ? value : (is_darkest ? std::min(extreme, value) : std::max(extreme, value));
    }

    return extreme;
}

// Parameter range along the line that lies inside the image, by clipping against each axis.
bool clip_to_image(const EdgeLine& line, const cv::Size& size, float& out_begin, float& out_end)
{
    float begin = -std::numeric_limits<float>::max();
    float end = std::numeric_limits<float>::max();

    const float origin[2] = {line.origin.x, line.origin.y};
    const float direction[2] = {line.direction.x, line.direction.y};
    const float limit[2] = {static_cast<float>(size.width - 1), static_cast<float>(size.height - 1)};

    for (int axis = 0; axis < 2; axis++)
    {
        if (std::abs(direction[axis]) < 1e-6F)
        {
            if (origin[axis] < 0.0F || origin[axis] > limit[axis])
            {
                return false;
            }
            continue;
        }

        float t0 = (0.0F - origin[axis]) / direction[axis];
        float t1 = (limit[axis] - origin[axis]) / direction[axis];
        if (t0 > t1)
        {
            std::swap(t0, t1);
        }

        begin = std::max(begin, t0);
        end = std::min(end, t1);
    }

    out_begin = begin;
    out_end = end;

    return end > begin;
}

// Samples both cues for both possible screen sides at every pixel along the line.
void measure_evidence(const cv::Mat& grayscale, EdgeLine& line)
{
    float t_end = 0.0F;
    if (!clip_to_image(line, grayscale.size(), line.t_begin, t_end))
    {
        return;
    }

    const int count = static_cast<int>(t_end - line.t_begin) + 1;

    line.valid_prefix.assign(count + 1, 0);
    for (auto& side : line.evidence)
    {
        side.rim_prefix.assign(count + 1, 0);
        side.glow_prefix.assign(count + 1, 0);
    }

    for (int index = 0; index < count; index++)
    {
        const cv::Point2f point = line.origin + line.direction * (line.t_begin + static_cast<float>(index));
        bool is_valid = true;

        for (int side = 0; side < 2; side++)
        {
            const cv::Point2f inward = (side == kPositiveSide) ? line.normal : -line.normal;

            const float rim = sample_extreme(grayscale, point, inward, kRimInsideOffsets, true);
            const float bezel = sample_extreme(grayscale, point, -inward, kRimOutsideOffsets, false);
            const float content = sample_extreme(grayscale, point, inward, kGlowInsideOffsets, false);
            const float darkness = sample_extreme(grayscale, point, -inward, kGlowOutsideOffsets, false);

            if (rim < 0.0F || bezel < 0.0F || content < 0.0F || darkness < 0.0F)
            {
                is_valid = false;
            }

            const bool has_rim = rim >= 0.0F && bezel > rim * kRimContrastRatio + kRimContrastMargin;
            const bool has_glow = darkness >= 0.0F
                                  && darkness < kGlowMaxOutside
                                  && content > darkness * kGlowContrastRatio + kGlowContrastMargin;

            line.evidence[side].rim_prefix[index + 1] = line.evidence[side].rim_prefix[index] + (has_rim ? 1 : 0);
            line.evidence[side].glow_prefix[index + 1] = line.evidence[side].glow_prefix[index] + (has_glow ? 1 : 0);
        }

        line.valid_prefix[index + 1] = line.valid_prefix[index] + (is_valid ? 1 : 0);
    }
}

// Fraction of the span between two points on the line that shows the screen border.
// Rim and glow are scored separately and the stronger one counts, so a side is never
// pieced together from unrelated edges.
double side_support(const EdgeLine& line, ScreenSide side, const cv::Point2f& from, const cv::Point2f& to)
{
    if (line.valid_prefix.empty())
    {
        return 0.0;
    }

    const int last = static_cast<int>(line.valid_prefix.size()) - 1;
    const auto to_index = [&](const cv::Point2f& point) {
        const float t = (point - line.origin).dot(line.direction) - line.t_begin;
        return std::clamp(static_cast<int>(std::lround(t)), 0, last);
    };

    int begin = to_index(from);
    int end = to_index(to);
    if (begin > end)
    {
        std::swap(begin, end);
    }

    const int span = end - begin;
    if (span <= 0)
    {
        return 0.0;
    }

    // A side running mostly off the image has too little evidence to trust.
    const int valid = line.valid_prefix[end] - line.valid_prefix[begin];
    if (valid * 2 < span)
    {
        return 0.0;
    }

    const SideEvidence& evidence = line.evidence[side];
    const int rim = evidence.rim_prefix[end] - evidence.rim_prefix[begin];
    const int glow = evidence.glow_prefix[end] - evidence.glow_prefix[begin];

    return static_cast<double>(std::max(rim, glow)) / span;
}

bool intersect(const EdgeLine& a, const EdgeLine& b, cv::Point2f& out_point)
{
    const float denominator = a.direction.x * b.direction.y - a.direction.y * b.direction.x;
    if (std::abs(denominator) < 1e-6F)
    {
        return false;
    }

    const cv::Point2f offset = b.origin - a.origin;
    const float t = (offset.x * b.direction.y - offset.y * b.direction.x) / denominator;
    out_point = a.origin + a.direction * t;

    return true;
}

bool is_duplicate(const EdgeLine& line, const std::vector<EdgeLine>& kept)
{
    const float max_cos = static_cast<float>(std::cos(kDuplicateLineAngleDegrees * CV_PI / 180.0));

    for (const auto& other : kept)
    {
        const float alignment = std::abs(line.direction.dot(other.direction));
        const float distance = std::abs((line.origin - other.origin).dot(other.normal));

        if (alignment >= max_cos && distance <= kDuplicateLineDistance)
        {
            return true;
        }
    }

    return false;
}

// Splits detected segments into near-horizontal and near-vertical lines, longest first,
// dropping collinear duplicates.
void collect_lines(const cv::Mat& grayscale,
                   std::vector<EdgeLine>& out_horizontal,
                   std::vector<EdgeLine>& out_vertical)
{
    cv::Mat blurred;
    cv::GaussianBlur(grayscale, blurred, cv::Size(3, 3), 0);

    std::vector<cv::Vec4f> segments;
    cv::createLineSegmentDetector(cv::LSD_REFINE_STD)->detect(blurred, segments);

    const float min_length = static_cast<float>(grayscale.cols * kMinLineLengthRatio);
    const double max_tilt = kMaxLineTiltDegrees;

    std::vector<EdgeLine> horizontal;
    std::vector<EdgeLine> vertical;

    for (const auto& segment : segments)
    {
        const cv::Point2f start(segment[0], segment[1]);
        const cv::Point2f end(segment[2], segment[3]);
        const cv::Point2f delta = end - start;
        const float length = std::hypot(delta.x, delta.y);

        if (length < min_length)
        {
            continue;
        }

        EdgeLine line;
        line.origin = (start + end) * 0.5F;
        line.direction = delta / length;
        line.length = length;

        const double angle = std::atan2(std::abs(line.direction.y), std::abs(line.direction.x)) * 180.0 / CV_PI;

        if (angle <= max_tilt)
        {
            // Point right, so the normal points down.
            if (line.direction.x < 0.0F)
            {
                line.direction = -line.direction;
            }
            line.normal = cv::Point2f(-line.direction.y, line.direction.x);
            horizontal.push_back(line);
        }
        else if (angle >= 90.0 - max_tilt)
        {
            // Point down, so the normal points right.
            if (line.direction.y < 0.0F)
            {
                line.direction = -line.direction;
            }
            line.normal = cv::Point2f(line.direction.y, -line.direction.x);
            vertical.push_back(line);
        }
    }

    const auto keep_longest = [&](std::vector<EdgeLine>& lines, std::vector<EdgeLine>& out_lines) {
        std::sort(lines.begin(), lines.end(), [](const EdgeLine& a, const EdgeLine& b) {
            return a.length > b.length;
        });

        for (auto& line : lines)
        {
            if (static_cast<int>(out_lines.size()) >= kMaxLinesPerOrientation)
            {
                break;
            }

            if (!is_duplicate(line, out_lines))
            {
                measure_evidence(grayscale, line);
                out_lines.push_back(std::move(line));
            }
        }
    };

    keep_longest(horizontal, out_horizontal);
    keep_longest(vertical, out_vertical);
}

} // namespace

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
    if (input_frame.empty() || input_frame.type() != CV_8UC3)
    {
        return false;
    }

    // The screen's border is a few pixels of dark rim; finding it on a downscaled copy keeps
    // the rim visible while bounding the cost on the Pi.
    const double scale = std::min(1.0, kWorkingWidth / input_frame.cols);

    cv::Mat working;
    cv::resize(input_frame, working, cv::Size(), scale, scale, cv::INTER_AREA);

    cv::Mat grayscale;
    cv::cvtColor(working, grayscale, cv::COLOR_BGR2GRAY);

    std::vector<EdgeLine> horizontal;
    std::vector<EdgeLine> vertical;
    collect_lines(grayscale, horizontal, vertical);

    const double width = grayscale.cols;
    const double height = grayscale.rows;
    const double frame_area = width * height;

    double best_score = 0.0;
    std::vector<cv::Point2f> best_corners;

    for (const auto& top : horizontal)
    {
        for (const auto& bottom : horizontal)
        {
            if (top.origin.y >= bottom.origin.y - height * kMinScreenSideRatio)
            {
                continue;
            }

            for (const auto& left : vertical)
            {
                for (const auto& right : vertical)
                {
                    if (left.origin.x >= right.origin.x - width * kMinScreenSideRatio)
                    {
                        continue;
                    }

                    std::vector<cv::Point2f> corners(4);
                    if (!intersect(top, left, corners[0]) || !intersect(top, right, corners[1])
                        || !intersect(bottom, right, corners[2]) || !intersect(bottom, left, corners[3]))
                    {
                        continue;
                    }

                    const double area_ratio = std::abs(cv::contourArea(corners)) / frame_area;
                    if (area_ratio < kMinScreenAreaRatio || area_ratio > kMaxScreenAreaRatio)
                    {
                        continue;
                    }

                    const double aspect_ratio = opencv_extensions::aspect_ratio(corners);
                    if (aspect_ratio < kMinScreenAspectRatio || aspect_ratio > kMaxScreenAspectRatio)
                    {
                        continue;
                    }

                    if (!cv::isContourConvex(corners))
                    {
                        continue;
                    }

                    const double supports[4] = {
                        side_support(top, kPositiveSide, corners[0], corners[1]),
                        side_support(right, kNegativeSide, corners[1], corners[2]),
                        side_support(bottom, kNegativeSide, corners[2], corners[3]),
                        side_support(left, kPositiveSide, corners[3], corners[0]),
                    };

                    const double weakest = *std::min_element(std::begin(supports), std::end(supports));
                    const double mean = (supports[0] + supports[1] + supports[2] + supports[3]) / 4.0;

                    // Every side must show the border: the weakest side keeps a quad with one
                    // invented edge from winning on the strength of the other three.
                    const double score = 0.5 * weakest + 0.5 * mean + kAreaScoreWeight * area_ratio;

                    if (score > best_score)
                    {
                        best_score = score;
                        best_corners = corners;
                    }
                }
            }
        }
    }

    if (best_corners.empty() || best_score < kMinScreenScore)
    {
        return false;
    }

    // Checked on the winner rather than filtered per candidate: dropping a cut-off screen
    // would hand the win to a smaller rectangle inside it, a confident wrong crop.
    const cv::Rect2f allowed(static_cast<float>(-width * kMaxCornerOverhangRatio),
                             static_cast<float>(-height * kMaxCornerOverhangRatio),
                             static_cast<float>(width * (1.0 + 2.0 * kMaxCornerOverhangRatio)),
                             static_cast<float>(height * (1.0 + 2.0 * kMaxCornerOverhangRatio)));

    for (const auto& corner : best_corners)
    {
        if (!allowed.contains(corner))
        {
            return false;
        }
    }

    for (auto& corner : best_corners)
    {
        corner *= static_cast<float>(1.0 / scale);
    }

    out_corners = best_corners;

    return true;
}

} // namespace screen_locator
} // namespace capture
} // namespace autoshinyhunthgss
