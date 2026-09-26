#ifndef CAPTURE_TEST_HELPERS_HPP
#define CAPTURE_TEST_HELPERS_HPP

#include <filesystem>
#include <iterator>
#include <random>
#include <string>
#include <system_error>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

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

} // namespace tests
} // namespace autoshinyhunthgss::capture

#endif // CAPTURE_TEST_HELPERS_HPP
