#ifndef CAPTURE_ENUMS_HPP
#define CAPTURE_ENUMS_HPP

#include <cstdint>
#include <string_view>

namespace autoshinyhunthgss {
namespace capture {

enum class CaptureStatus : std::uint8_t
{
    Unknown,
    ScreenFound,
    ScreenNotFound,
    Count
};

inline std::string_view to_string(CaptureStatus status)
{
    switch (status)
    {
        case CaptureStatus::ScreenFound:
            return "Screen Found";
        case CaptureStatus::ScreenNotFound:
            return "Screen Not Found";
        case CaptureStatus::Unknown:
        default:
            return "Unknown";
    }
}

enum class PixelFormat : std::uint8_t
{
    Unknown,
    Bgr888,
    Count
};

inline std::string_view to_string(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat::Bgr888:
            return "BGR888";
        case PixelFormat::Unknown:
        default:
            return "Unknown";
    }
}

inline int bytes_per_pixel(PixelFormat format)
{
    switch (format)
    {
        case PixelFormat::Bgr888:
            return 3;
        case PixelFormat::Unknown:
        default:
            return 0;
    }
}

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_ENUMS_HPP
