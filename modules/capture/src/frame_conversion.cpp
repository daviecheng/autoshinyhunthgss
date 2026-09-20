#include "frame_conversion.hpp"

#include <cstring>
#include <capture/capture_enums.hpp>

namespace autoshinyhunthgss {
namespace capture {

FrameConversionStatus fill_frame(const cv::Mat& screen,
                                std::int64_t timestamp_ns,
                                std::uint32_t sequence,
                                Frame& out_frame)
{
    if (screen.empty())
    {
        return FrameConversionStatus::EmptyImage;
    }

    if (screen.type() != CV_8UC3)
    {
        return FrameConversionStatus::UnsupportedPixelLayout;
    }

    if (screen.cols != kFrameWidth || screen.rows != kFrameHeight)
    {
        return FrameConversionStatus::UnsupportedDimensions;
    }

    const int row_bytes = kFrameWidth * bytes_per_pixel(kFramePixelFormat);

    out_frame.data.resize(static_cast<std::size_t>(row_bytes) * kFrameHeight);
    out_frame.width = kFrameWidth;
    out_frame.height = kFrameHeight;
    out_frame.stride = row_bytes;
    out_frame.format = kFramePixelFormat;
    out_frame.timestamp_ns = timestamp_ns;
    out_frame.sequence = sequence;

    for (int row = 0; row < kFrameHeight; row++)
    {
        std::memcpy(out_frame.data.data() + static_cast<std::size_t>(row) * row_bytes,
                    screen.ptr<std::uint8_t>(row),
                    static_cast<std::size_t>(row_bytes));
    }

    if (!has_expected_format(out_frame))
    {
        return FrameConversionStatus::FrameFormatViolation;
    }

    return FrameConversionStatus::Success;
}

} // namespace capture
} // namespace autoshinyhunthgss