#include <cstdlib>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <capture/capture_constants.hpp>

#include "screen_locator.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {

std::string fixture_path(const std::string& name)
{
    return std::string(CAPTURE_TEST_DATA_DIR) + "/" + name;
}

cv::Mat load_fixture(const std::string& name)
{
    const std::string path = fixture_path(name);
    if (!std::ifstream(path).good())
    {
        return cv::Mat{};
    }

    return cv::imread(path, cv::IMREAD_COLOR);
}

void dump_if_requested(const std::string& name, const cv::Mat& image)
{
    const char* directory = std::getenv("CAPTURE_TEST_DUMP_DIR");
    if (directory != nullptr && !image.empty())
    {
        cv::imwrite(std::string(directory) + "/" + name + ".png", image);
    }
}

TEST(ScreenLocatorTest, LocatesScreenInPhoto)
{
    const cv::Mat input_frame = load_fixture("screen_input.png");
    ASSERT_FALSE(input_frame.empty());

    ScreenLocator screenLocator;
    cv::Mat output_screen;

    const CaptureStatus status = screenLocator.locate(input_frame, output_screen);
    dump_if_requested("input", input_frame);
    dump_if_requested("output", output_screen);

    ASSERT_EQ(status, CaptureStatus::ScreenFound);
}

} // namespace tests
} // namespace autoshinyhunthgss::capture