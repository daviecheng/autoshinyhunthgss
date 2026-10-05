#include <gtest/gtest.h>

#include <capture/capture_constants.hpp>
#include <capture/capture_structs.hpp>

#include "frame_view.hpp"

namespace autoshinyhunthgss::vision {
namespace tests
{

using capture::Frame;
using capture::PixelFormat;

Frame make_frame(int width = capture::kFrameWidth,
                 int height = capture::kFrameHeight,
                 PixelFormat format = capture::kFramePixelFormat,
                 int stride_padding = 0)
{
    Frame frame;
    frame.width = width;
    frame.height = height;
    frame.format = format;
    frame.stride = width * 3 + stride_padding;
    frame.data.assign(static_cast<std::size_t>(frame.stride) * height, 0);

    return frame;
}

TEST(FrameViewTest, ValidFrameIsViewedWithoutCopy)
{
    const Frame frame = make_frame();

    const auto view = to_bgr_view(frame);

    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->cols, capture::kFrameWidth);
    EXPECT_EQ(view->rows, capture::kFrameHeight);
    EXPECT_EQ(view->type(), CV_8UC3);
    EXPECT_EQ(view->data, frame.data.data()); // same memory, no copy
}

TEST(FrameViewTest, WrongWidthIsRejected)
{
    EXPECT_FALSE(to_bgr_view(make_frame(600)).has_value());
}

TEST(FrameViewTest, WrongFormatIsRejected)
{
    const Frame frame = make_frame(capture::kFrameWidth, capture::kFrameHeight, PixelFormat::Unknown);

    EXPECT_FALSE(to_bgr_view(frame).has_value());
}

TEST(FrameViewTest, ShortDataIsRejected)
{
    Frame frame = make_frame();
    frame.data.resize(frame.data.size() - 1);

    EXPECT_FALSE(to_bgr_view(frame).has_value());
}

// A stride bug wouldn't crash: every row after the first would be sheared sieways and the
// bar and sprite measurements would drift. So write one pixel by stride and read it back by row.
TEST(FrameViewTest, PaddedStrideIsHonored)
{
    constexpr int kPadding = 16;
    constexpr int kRow = 10;
    constexpr int kCol = 20;
    Frame frame = make_frame(capture::kFrameWidth, capture::kFrameHeight, capture::kFramePixelFormat, kPadding);

    // offset = row * stride + col * 3
    const std::size_t offset = static_cast<std::size_t>(kRow) * frame.stride + kCol * 3;
    frame.data[offset + 0] = 11; // B
    frame.data[offset + 1] = 22; // G
    frame.data[offset + 2] = 33; // R

    const auto view = to_bgr_view(frame);

    ASSERT_TRUE(view.has_value());
    EXPECT_EQ(view->at<cv::Vec3b>(kRow, kCol), cv::Vec3b(11, 22, 33));
}

} // namespace tests
} // namespace autoshinyhunthgss::vision