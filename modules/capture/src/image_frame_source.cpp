#include "image_frame_source.hpp"

#include <chrono>
#include <iostream>

#include <opencv2/imgcodecs.hpp>

#include "frame_conversion.hpp"
#include "screen_locator.hpp"

namespace autoshinyhunthgss {
namespace capture {

namespace
{

std::int64_t now_ns()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();
}

} // namespace

ImageFrameSource::ImageFrameSource(const ImageSourceConfig& config)
    : _image_paths(config.image_paths)
    , _is_every_frame_retained(config.retention.is_every_frame_retained)
    , _frame_recorder(config.retention)
{}

CaptureStatus ImageFrameSource::capture_frame(Frame& out_frame)
{
    if (_next_index >= _image_paths.size())
    {
        return CaptureStatus::NoMoreFrames;
    }

    const std::string path = _image_paths[_next_index];
    const std::uint32_t sequence = static_cast<std::uint32_t>(_next_index);
    _next_index++;

    const cv::Mat input_frame = cv::imread(path, cv::IMREAD_COLOR);

    if (input_frame.empty())
    {
        std::cerr << "capture: cannot read " << path << "\n";
        return CaptureStatus::Unknown;
    }

    cv::Mat output_screen;
    const CaptureStatus status = screen_locator::locate(input_frame, output_screen);

    if (status != CaptureStatus::ScreenFound)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(status)));
        return status;
    }

    const FrameConversionStatus conversion = fill_frame(output_screen, now_ns(), sequence, out_frame);

    if (conversion != FrameConversionStatus::Success)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(conversion)));
        return CaptureStatus::Unknown;
    }

    if (_is_every_frame_retained)
    {
        _frame_recorder.record(input_frame, output_screen, std::string(to_string(status)));
    }

    return CaptureStatus::ScreenFound;
}

} // namespace capture
} // namespace autoshinyhunthgss