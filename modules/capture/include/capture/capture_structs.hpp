#ifndef CAPTURE_STRUCTS_HPP
#define CAPTURE_STRUCTS_HPP

#include <cstdint>
#include <vector>

#include <capture/capture_enums.hpp>
#include <capture/capture_constants.hpp>

namespace autoshinyhunthgss {
namespace capture {

struct Frame
{
    std::vector<std::uint8_t> data; // raw pixels of the screen
    int width = 0;
    int height = 0;
    int stride = 0; // bytes per row; row n starts at data[n * stride]
    PixelFormat format =PixelFormat::Unknown;
    std::int64_t timestamp_ns = 0; // sensor exposure time, may be useful later
    std::uint32_t sequence = 0; // camera frame counter, gaps indicate dropped frames
};

inline bool has_expected_format(const Frame& frame)
{
    return frame.width == kFrameWidth
        && frame.height == kFrameHeight
        && frame.format == kFramePixelFormat
        && frame.stride >= frame.width * bytes_per_pixel(frame.format)
        && frame.data.size() >= static_cast<std::size_t>(frame.stride) * frame.height;
}

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_STRUCTS_HPP