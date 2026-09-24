#include "frame_recorder.hpp"

#include <cctype>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <vector>

#include <opencv2/imgcodecs.hpp>

namespace autoshinyhunthgss {
namespace capture {

namespace {

constexpr int kSlotIndexDigits = 6;

// A 'reason' string is not guaranteed to be path safe, and spaces in directory
// names are awkward on a command line
std::string sanitize(const std::string& text)
{
    std::string result;
    result.reserve(text.size());

    for (const char character : text)
    {
        const bool is_safe = std::isalnum(static_cast<unsigned char>(character)) != 0;
        result.push_back(is_safe ? character : '_');
    }

    return result;
}

std::string make_slot_prefix(int index)
{
    char prefix[16];
    std::snprintf(prefix, sizeof(prefix), "%0*d_", kSlotIndexDigits, index);

    return prefix;
}

bool is_slot_directory(const std::string& name)
{
    if (static_cast<int>(name.size()) <= kSlotIndexDigits || name[kSlotIndexDigits] != '_')
    {
        return false;
    }

    for (int position = 0; position < kSlotIndexDigits; position++)
    {
        if (std::isdigit(static_cast<unsigned char>(name[position])) == 0)
        {
            return false;
        }
    }

    return true;
}

} // namespace

FrameRecorder::FrameRecorder(const FrameRetentionConfig& config)
    : _config(config)
{
    if (_config.output_directory.empty() || _config.max_retained_frames <= 0)
    {
        return;
    }

    std::error_code error;
    std::filesystem::create_directories(_config.output_directory, error);

    if (error)
    {
        std::cerr << "capture: cannot create " << _config.output_directory
                  << " -- " << error.message() << "; frame retention disabled\n";
        return;
    }

    _is_enabled = true;
    clear_slot_directories();
}

bool FrameRecorder::is_enabled() const
{
    return _is_enabled;
}

void FrameRecorder::record(const cv::Mat& input_frame,
                           const cv::Mat& output_frame,
                           const std::string& reason)
{
    if (!_is_enabled)
    {
        return;
    }

    const std::filesystem::path slot = std::filesystem::path(_config.output_directory)
                                       / (make_slot_prefix(_next_index) + sanitize(reason));
    
    std::error_code error;
    std::filesystem::create_directories(slot, error);

    if (error)
    {
        std::cerr << "capture: cannot create " << slot << " -- " << error.message() << "\n";
        return; //leave the index alone so the next record retries
    }

    if (_config.is_input_frame_saved)
    {
        write_image((slot / "input_frame.png").string(), input_frame);
    }

    if (_config.is_output_frame_saved)
    {
        write_image((slot / "output_frame.png").string(), output_frame);
    }

    remove_slot(_next_index - _config.max_retained_frames);
    _next_index++;
}

void FrameRecorder::clear_slot_directories() const
{
    std::error_code error;
    std::vector<std::filesystem::path> stale;

    for (const auto& entry : std::filesystem::directory_iterator(_config.output_directory, error))
    {
        if (is_slot_directory(entry.path().filename().string()))
        {
            stale.push_back(entry.path());
        }
    }

    for (const auto& path : stale)
    {
        std::filesystem::remove_all(path, error);
    }
}

void FrameRecorder::remove_slot(int index) const
{
    if (index < 0)
    {
        return;
    }

    const std::string prefix = make_slot_prefix(index);

    std::error_code error;
    std::vector<std::filesystem::path> stale;

    for (const auto& entry : std::filesystem::directory_iterator(_config.output_directory, error))
    {
        if (entry.path().filename().string().rfind(prefix, 0) == 0)
        {
            stale.push_back(entry.path());
        }
    }

    for (const auto& path: stale)
    {
        std::filesystem::remove_all(path, error);
    }
}

void FrameRecorder::write_image(const std::string& path, const cv::Mat& image)
{
    if (image.empty())
    {
        return;
    }

    bool is_written = false;

    // A full storage must not take the hunt down, and imwrite can throw as well as return false
    try
    {
        is_written = cv::imwrite(path, image);
    }
    catch (const cv::Exception& exception)
    {
        std::cerr << "capture: imwrite threw for " << path
                  << " -- " << exception.what() << "\n";
    }

    if (!is_written && !_has_reported_write_failure)
    {
        std::cerr << "capture: failed to write " << path
                  << "; further write failures will not be reported\n";
        _has_reported_write_failure = true;
    }
}


    
} // namespace capture
} // namespace autoshinyhunthgss