#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include <capture/capture_constants.hpp>
#include <capture/capture_structs.hpp>

#include "frame_conversion.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {

TEST(FrameConversionTest, FillFrameWithScreen)
{
    const cv::Mat screen(kFrameHeight, kFrameWidth, CV_8UC3, cv::Scalar(1, 2, 3));

    Frame frame;
    ASSERT_EQ(fill_frame(screen, 123, 7, frame), FrameConversionStatus::Success);

    EXPECT_TRUE(has_expected_format(frame));
    EXPECT_EQ(frame.data[0], 1); // B
    EXPECT_EQ(frame.data[1], 2); // G
    EXPECT_EQ(frame.data[2], 3); // R
    EXPECT_EQ(frame.width, kFrameWidth);
    EXPECT_EQ(frame.height, kFrameHeight);
    EXPECT_EQ(frame.timestamp_ns, 123);
    EXPECT_EQ(frame.sequence, 7u);
}

TEST(FrameConversionTest, FillFrameRejectsInvalidScreenSize)
{
    int invalidScreenHeight = 100;
    int invalidScreenWidth = 50;
    const cv::Mat screen(invalidScreenHeight, invalidScreenWidth, CV_8UC3, cv::Scalar::all(0));

    Frame frame;
    EXPECT_EQ(fill_frame(screen, 0, 0, frame), FrameConversionStatus::UnsupportedDimensions);
}

TEST(FrameConversionTest, FillFrameRejectsEmptyScreen)
{
    Frame frame;
    EXPECT_EQ(fill_frame(cv::Mat{}, 0, 0, frame), FrameConversionStatus::EmptyImage);
}

} // namespace tests
} // namespace autoshinyhunthgss::capture