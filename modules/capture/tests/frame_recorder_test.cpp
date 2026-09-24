#include <filesystem>
#include <iterator>
#include <random>
#include <string>
#include <system_error>

#include <gtest/gtest.h>
#include <opencv2/core.hpp>

#include <capture/capture_constants.hpp>
#include <capture/frame_source_factory.hpp>

#include "frame_recorder.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {
namespace {

constexpr const char* kTestName = "frame_recorder";

// Cleans on entry, not on exit, so the last run's folders stay behind to look at.
std::filesystem::path make_case_directory(const std::string& case_name)
{
    const std::filesystem::path directory =
        std::filesystem::path(CAPTURE_TEST_OUTPUT_DIR) / kTestName / case_name;

    std::error_code error;
    std::filesystem::remove_all(directory, error);

    return directory;
}

std::size_t count_entries(const std::filesystem::path& directory)
{
    std::error_code error;

    return static_cast<std::size_t>(std::distance(std::filesystem::directory_iterator(directory, error),
                                                  std::filesystem::directory_iterator{}));
}

FrameRetentionConfig make_config(const std::filesystem::path& directory, int max_retained_frames)
{
    FrameRetentionConfig config;
    config.output_directory = directory.string();
    config.max_retained_frames = max_retained_frames;

    return config;
}

cv::Scalar make_random_color()
{
    static std::mt19937 engine(42);
    std::uniform_int_distribution<int> channel(0, 255);

    return cv::Scalar(channel(engine), channel(engine), channel(engine));
}

cv::Mat make_input_frame()
{
    return cv::Mat(64, 64, CV_8UC3, make_random_color());
}

cv::Mat make_output_frame()
{
    return cv::Mat(kFrameHeight, kFrameWidth, CV_8UC3, make_random_color());
}

void verify_frame_capture(const std::filesystem::path& slot)
{
    EXPECT_TRUE(std::filesystem::is_directory(slot));
    EXPECT_TRUE(std::filesystem::exists(slot / "input_frame.png"));
    EXPECT_TRUE(std::filesystem::exists(slot / "output_frame.png"));
}

} // namespace

TEST(FrameRecorderTest, RecordOneFrame)
{
    const auto directory = make_case_directory("record_one_frame");

    FrameRecorder recorder(make_config(directory, 10));
    ASSERT_TRUE(recorder.is_enabled());

    recorder.record(make_input_frame(), make_output_frame(), "Screen Not Found");

    verify_frame_capture(directory / "000000_Screen_Not_Found");
}

TEST(FrameRecorderTest, RecordTwoFrames)
{
    const auto directory = make_case_directory("record_two_frames");

    FrameRecorder recorder(make_config(directory, 5));
    ASSERT_TRUE(recorder.is_enabled());

    recorder.record(make_input_frame(), make_output_frame(), "Empty Image");
    recorder.record(make_input_frame(), make_output_frame(), "Screen Not Found");

    verify_frame_capture(directory / "000000_Empty_Image");
    verify_frame_capture(directory / "000001_Screen_Not_Found");
}

TEST(FrameRecorderTest, OldestSlotRemovedWhenMaxSlotsReached)
{
    const auto directory = make_case_directory("oldest_slot_removed");

    FrameRecorder recorder(make_config(directory, 1));
    ASSERT_TRUE(recorder.is_enabled());

    recorder.record(make_input_frame(), make_output_frame(), "Empty Image");
    verify_frame_capture(directory / "000000_Empty_Image");

    recorder.record(make_input_frame(), make_output_frame(), "Screen Not Found");
    verify_frame_capture(directory / "000001_Screen_Not_Found");

    EXPECT_EQ(count_entries(directory), 1u);
}

TEST(FrameRecorderTest, ClearsPreviousSessionOnStart)
{
    const auto directory = make_case_directory("restart");

    {
        FrameRecorder first(make_config(directory, 10));
        first.record(make_input_frame(), make_output_frame(), "Screen Not Found");
        first.record(make_input_frame(), make_output_frame(), "Unknown");

        verify_frame_capture(directory / "000000_Screen_Not_Found");
        verify_frame_capture(directory / "000001_Unknown");
    }

    FrameRecorder second(make_config(directory, 10));
    EXPECT_EQ(count_entries(directory), 0u);

    second.record(make_input_frame(), make_output_frame(), "Unknown");
    EXPECT_TRUE(std::filesystem::is_directory(directory / "000000_Unknown"));
}

TEST(FrameRecorderTest, RecorderDisabledWhenNoOutputDirectory)
{
    std::filesystem::path empty_directory;
    empty_directory.clear();

    FrameRecorder recorder(make_config(empty_directory, 10));
    EXPECT_FALSE(recorder.is_enabled());
}

TEST(FrameRecorderTest, RecorderDisabledWhenZeroRetentionCount)
{
    std::filesystem::path directory("Zero Retention");

    FrameRecorder recorder(make_config(directory, 0));
    EXPECT_FALSE(recorder.is_enabled());
}


} // namespace tests
} // namespace autoshinyhunthgss::capture