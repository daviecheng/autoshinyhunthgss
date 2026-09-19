#ifndef CAPTURE_I_FRAME_SOURCE_HPP
#define CAPTURE_I_FRAME_SOURCE_HPP

#include <capture/capture_enums.hpp>
#include <capture/capture_structs.hpp>

namespace autoshinyhunthgss {
namespace capture {

class IFrameSource
{
public:
    virtual ~IFrameSource() = default;
    virtual CaptureStatus capture_frame(Frame& out_frame) = 0;
};

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_I_FRAME_SOURCE_HPP