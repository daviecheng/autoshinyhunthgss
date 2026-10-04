#include <algorithm>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

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

// Every image in one test_data subdirectory, sorted so frame sequence matches file order.
std::vector<std::string> list_dataset_images(const std::string& dataset)
{
    std::vector<std::string> paths;
    std::error_code error;

    for (const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(TEST_DATA_DIR) / dataset, error))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".png")
        {
            paths.push_back(entry.path().string());
        }
    }

    std::sort(paths.begin(), paths.end());

    return paths;
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

// One test_data subdirectory and the status every image in it should produce.
struct DatasetCase
{
    std::string dataset;
    CaptureStatus expected_status;
};

// "night/both_health_bars" -> "night_both_health_bars", usable as a test and folder name.
std::string to_case_name(const std::string& dataset)
{
    std::string name = dataset;
    std::replace(name.begin(), name.end(), '/', '_');

    return name;
}

class ImageFrameSourceDatasetTest : public ::testing::TestWithParam<DatasetCase>
{
};

// Runs a whole rig dataset through the source and keeps every frame, each dataset in its own
// test_output folder, so the crops can be inspected after a locator change.
TEST_P(ImageFrameSourceDatasetTest, EveryFrameRetained)
{
    const DatasetCase& dataset_case = GetParam();

    const auto test_dir = make_test_directory(kTestName, "dataset_" + to_case_name(dataset_case.dataset));
    const auto retention_dir = test_dir / "retained";

    ImageSourceConfig config;
    config.image_paths = list_dataset_images(dataset_case.dataset);
    ASSERT_FALSE(config.image_paths.empty()) << "no images in " << dataset_case.dataset;

    const int num_images = static_cast<int>(config.image_paths.size());

    config.retention.output_directory = retention_dir.string();
    config.retention.max_retained_frames = num_images;
    config.retention.is_every_frame_retained = true;

    ImageFrameSource source(config);

    Frame frame;
    for (int index = 0; index < num_images; index++)
    {
        EXPECT_EQ(source.capture_frame(frame), dataset_case.expected_status) << config.image_paths[index];
    }

    EXPECT_EQ(source.capture_frame(frame), CaptureStatus::NoMoreFrames);
    EXPECT_EQ(count_entries(retention_dir), static_cast<std::size_t>(num_images));
}

INSTANTIATE_TEST_SUITE_P(
    TestData,
    ImageFrameSourceDatasetTest,
    ::testing::Values(
        DatasetCase{"day/both_health_bars", CaptureStatus::ScreenFound},
        DatasetCase{"day/one_health_bar", CaptureStatus::ScreenFound},
        DatasetCase{"night/both_health_bars", CaptureStatus::ScreenFound},
        DatasetCase{"night/one_health_bar", CaptureStatus::ScreenFound},
        DatasetCase{"non_encounter", CaptureStatus::ScreenFound},
        DatasetCase{"screen_found", CaptureStatus::ScreenFound},
        DatasetCase{"screen_not_found", CaptureStatus::ScreenNotFound},
        DatasetCase{"sunset/both_health_bars", CaptureStatus::ScreenFound},
        DatasetCase{"sunset/one_health_bar", CaptureStatus::ScreenFound}),
    [](const ::testing::TestParamInfo<DatasetCase>& info) { return to_case_name(info.param.dataset); });

} // namespace tests
} // namespace autoshinyhunthgss::capture