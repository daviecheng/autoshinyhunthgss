#ifndef VISION_TARGET_PROFILES_HPP
#define VISION_TARGET_PROFILES_HPP

#include "vision_config.hpp"

namespace autoshinyhunthgss {
namespace vision {

inline TargetProfile ho_oh_profile()
{
    // Normal:          ratio 1.76 - 3.91   , gold 0.036 - 0.066
    // Synthetic shiny: ratio <= 0.80       , gold >= 0.129

    TargetProfile profile;
    profile.sprite_roi = cv::Rect(285, 0, 205, 165);
    profile.sprite_s_min = 90;
    profile.sprite_v_min = 70;
    profile.normal_band = HsvBand{176, 7, 90, 70};
    profile.shiny_band = HsvBand{14, 34, 90, 70};
    profile.shiny_max_ratio = 1.0;
    profile.shiny_min_fraction = 0.11;
    profile.normal_min_ratio = 1.5;
    profile.normal_max_fraction = 0.09;

    return profile;
}

} // namespace vision
} // namespace autoshinyhunthgss

#endif // VISION_TARGET_PROFILES_HPP