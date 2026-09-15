#ifndef FRAME_SOURCE_HPP
#define FRAME_SOURCE_HPP

#include <capture/capture_enums.hpp>

namespace autoshinyhunthgss {
namespace capture {

class FrameSource
{
public:
    CaptureStatus capture_frame(bool is_device_available) const;
};

} // namespace capture
} // namespace autoshinyhunthgss

#endif
