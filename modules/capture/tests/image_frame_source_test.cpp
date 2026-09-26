#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <capture/capture_constants.hpp>
#include <capture/capture_structs.hpp>

#include "image_frame_source.hpp"
#include "test_helpers.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {
namespace {

constexpr const char* kTestName = "image_frame_source";

// One flat tone, so no region separates from any other at any threshold.
std::string write_blank_image(const std::filesystem::path& directory, const std::string& name)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);

    const cv::Mat image(600, 800, CV_8UC3, cv::Scalar::all(0));

    const std::string path = (directory / name).string();
    cv::imwrite(path, image);

    return path;
}

} // namespace

TEST(ImageFrameSourceTest, ScreenNotFoundIsRetained)
{
    const auto test_dir = make_test_directory(kTestName, "screen_not_found_retained");
    const auto retention_dir = test_dir / "retained";

    ImageSourceConfig config;
    config.image_paths = {write_blank_image(test_dir, "blank_image.png")};
    config.retention.output_directory = retention_dir.string();

    ImageFrameSource source(config);

    Frame frame;
    ASSERT_EQ(source.capture_frame(frame), CaptureStatus::ScreenNotFound);
    EXPECT_EQ(source.capture_frame(frame), CaptureStatus::NoMoreFrames);

    EXPECT_EQ(count_entries(retention_dir), 1u);

    const auto slot = retention_dir / "000000_Screen_Not_Found";
    EXPECT_TRUE(std::filesystem::exists(slot / "input_frame.png"));
    EXPECT_FALSE(std::filesystem::exists(slot / "output_frame.png"));
}

TEST(ImageFrameSourceTest, ScreenFoundNoRetention)
{
    const auto test_dir = make_test_directory(kTestName, "screens_found_no_retention");
    const int num_images = 20;

    ImageSourceConfig config;
    config.image_paths = write_locatable_images(test_dir, num_images);

    ImageFrameSource source(config);

    Frame frame;
    for (int index = 0; index < num_images; index++)
    {
        ASSERT_EQ(source.capture_frame(frame), CaptureStatus::ScreenFound) << "at image " << index;
        EXPECT_EQ(frame.sequence, static_cast<std::uint32_t>(index));
    }

    EXPECT_EQ(source.capture_frame(frame), CaptureStatus::NoMoreFrames);
}

TEST(ImageFrameSourceTest, ScreenFoundWithRetention)
{
    const auto test_dir = make_test_directory(kTestName, "screens_found_with_retention");
    const auto retention_dir = test_dir / "retained";
    const int num_images = 10;
    const int max_retained_frames = num_images;

    ImageSourceConfig config;
    config.image_paths = write_locatable_images(test_dir, num_images);
    config.retention.output_directory = retention_dir.string();
    config.retention.max_retained_frames = max_retained_frames;
    config.retention.is_every_frame_retained = true;

    ImageFrameSource source(config);

    Frame frame;
    for (int index = 0; index < num_images; index++)
    {
        ASSERT_EQ(source.capture_frame(frame), CaptureStatus::ScreenFound) << "at image " << index;
        EXPECT_EQ(frame.sequence, static_cast<std::uint32_t>(index));
    }

    EXPECT_EQ(source.capture_frame(frame), CaptureStatus::NoMoreFrames);
    EXPECT_EQ(count_entries(retention_dir), static_cast<std::size_t>(max_retained_frames));
}

TEST(ImageFrameSourceTest, ScreenFoundWithRetentionRing)
{
    const auto test_dir = make_test_directory(kTestName, "retention_ring");
    const auto retention_dir = test_dir / "retained";
    const int num_images = 5;
    const int max_retained_frames = 3;

    ImageSourceConfig config;
    config.image_paths = write_locatable_images(test_dir, num_images);
    config.retention.output_directory = retention_dir.string();
    config.retention.max_retained_frames = max_retained_frames;
    config.retention.is_every_frame_retained = true;

    ImageFrameSource source(config);

    Frame frame;
    for (int index = 0; index < num_images; index++)
    {
        ASSERT_EQ(source.capture_frame(frame), CaptureStatus::ScreenFound) << "at image " << index;
        EXPECT_EQ(frame.sequence, static_cast<std::uint32_t>(index));
    }

    EXPECT_EQ(source.capture_frame(frame), CaptureStatus::NoMoreFrames);
    EXPECT_EQ(count_entries(retention_dir), static_cast<std::size_t>(max_retained_frames));
}

} // namespace tests
} // namespace autoshinyhunthgss::capture