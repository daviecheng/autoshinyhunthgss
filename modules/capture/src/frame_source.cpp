#include "frame_source.hpp"

namespace autoshinyhunthgss {
namespace capture {

CaptureStatus FrameSource::capture_frame(bool is_device_available) const
{
    if (is_device_available)
    {
        return CaptureStatus::Ok;
    }

    return CaptureStatus::NoDevice;
}

} // namespace capture
} // namespace autoshinyhunthgss
