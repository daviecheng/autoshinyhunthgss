#ifndef CAPTURE_ENUMS_HPP
#define CAPTURE_ENUMS_HPP

#include <cstdint>
#include <string_view>

namespace autoshinyhunthgss {
namespace capture {

enum class CaptureStatus : std::uint8_t {
    Unknown,
    Ok,
    NoDevice,
    ConfigurationFailed,
    Timeout,
    Count
};

inline std::string_view to_string(CaptureStatus status)
{
    switch (status) {
        case CaptureStatus::Ok:
            return "Ok";
        case CaptureStatus::NoDevice:
            return "NoDevice";
        case CaptureStatus::ConfigurationFailed:
            return "ConfigurationFailed";
        case CaptureStatus::Timeout:
            return "Timeout";
        case CaptureStatus::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace capture
} // namespace autoshinyhunthgss

#endif //CAPTURE_ENUMS_HPP
