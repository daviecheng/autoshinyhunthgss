#ifndef CAPTURE_TEST_HELPERS_HPP
#define CAPTURE_TEST_HELPERS_HPP

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <gtest/gtest.h>
#include <iterator>
#include <random>
#include <string>
#include <system_error>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace autoshinyhunthgss::capture {
namespace tests {

// Cleans on entry, not on exit, so the last run's folders stay behind to look at.
inline std::filesystem::path make_test_directory(const std::string& test_name,
                                                 const std::string& case_name)
{
    const std::filesystem::path directory =
        std::filesystem::path(CAPTURE_TEST_OUTPUT_DIR) / test_name / case_name;

    std::error_code error;
    std::filesystem::remove_all(directory, error);

    return directory;
}

inline std::size_t count_entries(const std::filesystem::path& directory)
{
    std::error_code error;

    return static_cast<std::size_t>(std::distance(std::filesystem::directory_iterator(directory, error),
                                                  std::filesystem::directory_iterator{}));
}

inline std::string test_data_path(const std::string& test_name, const std::string& file_name)
{
    return (std::filesystem::path(CAPTURE_TEST_DATA_DIR) / test_name / file_name).string();
}

inline cv::Mat load_artifact(const std::string& test_name, const std::string& name)
{
    const std::filesystem::path path = std::filesystem::path(CAPTURE_TEST_DATA_DIR) / test_name / name;

    if (!std::filesystem::exists(path))
    {
        return cv::Mat{};
    }

    return cv::imread(path.string(), cv::IMREAD_COLOR);
}

inline void write_artifact(const std::string& test_name, const std::string& name, const cv::Mat& image)
{
    if (image.empty())
    {
        return;
    }

    const std::filesystem::path directory = std::filesystem::path(CAPTURE_TEST_OUTPUT_DIR) / test_name;

    std::error_code error;
    std::filesystem::create_directories(directory, error);

    if (error)
    {
        return;
    }

    cv::imwrite((directory / (name + ".png")).string(), image);
}

inline cv::Scalar make_random_color()
{
    static std::mt19937 engine(42);
    std::uniform_int_distribution<int> channel(0, 255);

    return cv::Scalar(channel(engine), channel(engine), channel(engine));
}

// Helpers to be called during development to help create mock images to be added to test_data.
namespace {
const cv::Size kMockImageSize(800, 600);
const cv::Size kMockScreenSize(400, 300);

inline bool is_under_test_output(const std::filesystem::path& path)
{
    std::error_code error;

    const auto root = std::filesystem::weakly_canonical(CAPTURE_TEST_OUTPUT_DIR, error);
    if (error)
    {
        return false;
    }

    const auto target = std::filesystem::weakly_canonical(path, error);
    if (error)
    {
        return false;
    }

    const auto shared = std::mismatch(root.begin(), root.end(), target.begin(), target.end());

    return shared.first == root.end();
}

inline void clear_test_directory(const std::filesystem::path& directory)
{
    if (!is_under_test_output(directory))
    {
        ADD_FAILURE() << directory << " is outside " << CAPTURE_TEST_OUTPUT_DIR
                      << "; unable to delete";
        return;
    }

    std::error_code error;
    std::filesystem::remove_all(directory, error);
}

// Fixed seed: each mock lands somewhere different so they can be told apart by
// eye, but the sequence is the same every run so a failure reproduces. Local to
// this file rather than the shared helper, so another test file drawing from
// the shared engine cannot shift these.
inline cv::Point make_random_origin()
{
    static std::mt19937 engine(7);

    std::uniform_int_distribution<int> x(0, kMockImageSize.width - kMockScreenSize.width);
    std::uniform_int_distribution<int> y(0, kMockImageSize.height - kMockScreenSize.height);

    return cv::Point(x(engine), y(engine));
}

// A bright 4:3 rectangle on black. The locator finds the screen as a filled
// region, so the mock only has to be a screen-shaped block that separates from
// its background by brightness. A fixed color, not a random one: a dark fill
// on a dark background is correctly not locatable.
inline std::string write_locatable_image(const std::filesystem::path& directory, const std::string& name)
{
    std::error_code error;
    std::filesystem::create_directories(directory, error);

    cv::Mat image(kMockImageSize, CV_8UC3, cv::Scalar::all(0));
    cv::rectangle(image, cv::Rect(make_random_origin(), kMockScreenSize), cv::Scalar::all(255), cv::FILLED);

    const std::string path = (directory / name).string();
    cv::imwrite(path, image);

    return path;
}

inline std::vector<std::string> write_locatable_images(const std::filesystem::path& directory, int count)
{
    const std::filesystem::path mocks = directory / "mock_screens";
    clear_test_directory(mocks);

    std::vector<std::string> paths;
    paths.reserve(static_cast<std::size_t>(count));

    for (int index = 0; index < count; index++)
    {
        char name[32];
        std::snprintf(name, sizeof(name), "%03d_mock.png", index);

        paths.push_back(write_locatable_image(mocks, name));
    }

    return paths;
}

} // namespace

} // namespace tests
} // namespace autoshinyhunthgss::capture

#endif // CAPTURE_TEST_HELPERS_HPP
