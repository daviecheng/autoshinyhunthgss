#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <capture/capture_constants.hpp>

#include "screen_locator.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {

constexpr const char* kTestName = "screen_locator";

cv::Mat load_artifact(const std::string& name)
{
    const std::filesystem::path path = std::filesystem::path(CAPTURE_TEST_DATA_DIR) / kTestName / name;

    if (!std::filesystem::exists(path))
    {
        return cv::Mat{};
    }

    return cv::imread(path.string(), cv::IMREAD_COLOR);
}

void write_artifact(const std::string& name, const cv::Mat& image)
{
    if (image.empty())
    {
        return;
    }

    const std::filesystem::path directory = std::filesystem::path(CAPTURE_TEST_OUTPUT_DIR) / kTestName;

    std::error_code error;
    std::filesystem::create_directories(directory, error);

    if (error)
    {
        return;
    }

    cv::imwrite((directory / (name + ".png")).string(), image);
}

TEST(ScreenLocatorTest, LocatesScreenInPhoto)
{
    const cv::Mat input_frame = load_artifact("screen_input.png");
    ASSERT_FALSE(input_frame.empty());

    ScreenLocator screenLocator;
    cv::Mat output_screen;

    const CaptureStatus status = screenLocator.locate(input_frame, output_screen);
    write_artifact("input_frame", input_frame);
    write_artifact("output_frame", output_screen);

    ASSERT_EQ(status, CaptureStatus::ScreenFound);
}

} // namespace tests
} // namespace autoshinyhunthgss::capture