#ifndef CAPTURE_ENUMS_P_HPP
#define CAPTURE_ENUMS_P_HPP

#include <cstdint>
#include <string_view>

namespace autoshinyhunthgss {
namespace capture {

enum class FrameConversionStatus : std::uint8_t
{
    Unknown,
    Success,
    EmptyImage,
    UnsupportedPixelLayout,
    UnsupportedDimensions,
    FrameFormatViolation,
    Count
};

inline std::string_view to_string(FrameConversionStatus status)
{
    switch (status)
    {
        case FrameConversionStatus::Success:
            return "Success";
        case FrameConversionStatus::EmptyImage:
            return "Empty Image";
        case FrameConversionStatus::UnsupportedPixelLayout:
            return "Unsupported Pixel Layout";
        case FrameConversionStatus::UnsupportedDimensions:
            return "Unsupported Dimensions";
        case FrameConversionStatus::FrameFormatViolation:
            return "Frame Format Violation";
        case FrameConversionStatus::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_ENUMS_P_HPP
