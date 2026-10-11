#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "health_bar_detector.hpp"

namespace autoshinyhunthgss::vision {
namespace tests {

constexpr int kGapTolerance = 2;

// Sets 'length' pixels of 'row' to 255, starting at 'start'
void set_run(cv::Mat&mask, int row, int start, int length)
{
    mask(cv::Rect(start, row, length, 1)).setTo(255);
}

cv::Mat make_mask(int rows = 2, int cols = 20)
{
    return cv::Mat::zeros(rows, cols, CV_8U);
}

TEST(LongestRunTest, EmptyMaskHasNoRun)
{
    EXPECT_EQ(longest_run(make_mask(), kGapTolerance), 0);
}

TEST(LongestRunTest, SolidRunIsCountedInFull)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 3, 10);

    EXPECT_EQ(longest_run(mask, kGapTolerance), 10);
}

// A gap within tolerance is camera noise. The run continues and gap counts toward it.
TEST(LongestRunTest, GapWithinToleranceIsBridged)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 0, 4); // 4 Whites (cols 0-3), 2 gaps (cols 4-5)
    set_run(mask, 0, 6, 3); // 3 Whites (cols 6-8)

    EXPECT_EQ(longest_run(mask, kGapTolerance), 9);
}

TEST(LongestRunTest, GapOverToleranceBreaksRun)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 0, 4); // 4 Whites (cols 0-3), 3 Gaps (cols 4-6)
    set_run(mask, 0, 7, 3); // 3 Whites (cols 7-8)

    EXPECT_EQ(longest_run(mask, kGapTolerance), 4);
}

TEST(LongestRunTest, RunsOnDifferentRowsAreNotCombined)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 10, 5); // 5 Whites (row 0: cols 10 -14)
    set_run(mask, 1, 2, 5); // 5 Whites (row 1: cols 2 - 6)

    EXPECT_EQ(longest_run(mask, kGapTolerance), 5);
}

TEST(LongestRunTest, LongestRowWins)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 10, 5); // 5 Whites (row 0: cols 10 -14)
    set_run(mask, 1, 2, 4); // 5 Whites (row 1: cols 2 - 6)

    EXPECT_EQ(longest_run(mask, kGapTolerance), 5);
}

TEST(LongestRunTest, RunTouchingBothEdgesIsCountedInFull)
{
    cv::Mat mask = make_mask();
    set_run(mask, 0, 0, mask.cols);

    EXPECT_EQ(longest_run(mask, kGapTolerance), mask.cols);
}

} // namespace tests
} // namespace autoshinyhunthgss::vision
