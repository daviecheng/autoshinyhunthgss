#include "vision_config.hpp"
#include "target_profiles.hpp"

namespace autoshinyhunthgss {
namespace vision {

std::optional<TargetProfile> target_profile(HuntTarget target)
{
    switch (target)
    {
        case HuntTarget::HoOh:
            return ho_oh_profile();
        default:
            return std::nullopt;
    }
}

} // namespace vision
} // namespace autoshinyhunthgss