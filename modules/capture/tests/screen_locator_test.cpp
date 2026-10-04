#include <filesystem>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <capture/capture_constants.hpp>

#include "screen_locator.hpp"
#include "test_helpers.hpp"

namespace autoshinyhunthgss::capture {
namespace tests {
namespace {
constexpr const char* kTestName = "screen_locator";
} // namespace

// TEST(ScreenLocatorTest, LocatesScreenInPhoto)
// {
//     const cv::Mat input_frame = load_artifact("night/both_health_bars", "001.png");
//     ASSERT_FALSE(input_frame.empty());

//     cv::Mat output_screen;

//     const CaptureStatus status = screen_locator::locate(input_frame, output_screen);
//     write_artifact(kTestName, "input_frame", input_frame);
//     write_artifact(kTestName, "output_frame", output_screen);

//     ASSERT_EQ(status, CaptureStatus::ScreenFound);
// }

} // namespace tests
} // namespace autoshinyhunthgss::capture