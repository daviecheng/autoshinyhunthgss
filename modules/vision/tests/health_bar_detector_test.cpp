#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <capture/frame_source_factory.hpp>

#include "frame_view.hpp"
#include "health_bar_detector.hpp"

namespace autoshinyhunthgss::vision {
namespace tests {

namespace {
    constexpr int kGapTolerance = 2;
    const std::vector<std::string> kTimesOfDay{"day", "sunset", "night"};

    // Sets 'length' pixels of 'row' to 255, starting at 'start'
    void set_run(cv::Mat&mask, int row, int start, int length)
    {
        mask(cv::Rect(start, row, length, 1)).setTo(255);
    }

    cv::Mat make_mask(int rows = 2, int cols = 20)
    {
        return cv::Mat::zeros(rows, cols, CV_8U);
    }

    // Every .png in a test_data subdirectory, sorted so failures are reported in file order.
    std::vector<std::string> list_images(const std::string& dataset)
    {
        std::vector<std::string> paths;

        for (const auto& entry: std::filesystem::directory_iterator(std::filesystem::path(TEST_DATA_DIR) / dataset))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".png")
            {
                paths.push_back(entry.path().string());
            }
        }

        std::sort(paths.begin(), paths.end());

        return paths;
    }

    std::optional<HealthBars> read_health_bars(const std::string& path)
    {
        capture::ImageSourceConfig config;
        config.image_paths = {path};
        const auto source = capture::make_image_frame_source(config);

        capture::Frame frame;
        if (source->capture_frame(frame) != capture::CaptureStatus::ScreenFound)
        {
            return std::nullopt;
        }

        const auto bgr = to_bgr_view(frame);
        if (!bgr.has_value())
        {
            return std::nullopt;
        }

        cv::Mat hsv;
        cv::cvtColor(*bgr, hsv, cv::COLOR_BGR2HSV);

        return detect_health_bars(hsv, SingleBattleUiConfig{});
    }
} // namespace

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

TEST(HealthBarDetectorTest, SettledEncounterShowsBothBars)
{
    for (const auto& time_of_day : kTimesOfDay)
    {
        const auto paths = list_images(time_of_day + "/both_health_bars");
        ASSERT_FALSE(paths.empty()) << time_of_day;

        for (const auto& path: paths)
        {
            const auto bars = read_health_bars(path);
            ASSERT_TRUE(bars.has_value()) << path;
            EXPECT_TRUE(bars->has_enemy_bar) << path << " enemy run " << bars->enemy_run;
            EXPECT_TRUE(bars->has_player_bar) << path << " player run " << bars->player_run;
        }
    }
}

TEST(HealthBarDetectorTest, EncounterIntroNeverShowsPlayerBar)
{
    for (const auto& time_of_day : kTimesOfDay)
    {
        const auto paths = list_images(time_of_day + "/one_health_bar");
        ASSERT_FALSE(paths.empty()) << time_of_day;

        for (const auto& path: paths)
        {
            const auto bars = read_health_bars(path);
            ASSERT_TRUE(bars.has_value()) << path;
            EXPECT_FALSE(bars->has_player_bar) << path << " player run " << bars->player_run;
        }
    }
}

TEST(HealthBarDetectorTest, NonEncounterShowsNoBars)
{
    const auto paths = list_images("non_encounter");
    ASSERT_FALSE(paths.empty());

    for (const auto& path: paths)
    {
        const auto bars = read_health_bars(path);
        ASSERT_TRUE(bars.has_value()) << path;
        EXPECT_FALSE(bars->has_enemy_bar) << path << " enemy run " << bars->enemy_run;
        EXPECT_FALSE(bars->has_player_bar) << path << " player run " << bars->player_run;
    }
}

} // namespace tests
} // namespace autoshinyhunthgss::vision
