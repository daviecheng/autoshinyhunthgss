#ifndef VISION_CONFIG_HPP
#define VISION_CONFIG_HPP

#include <optional>
#include <opencv2/core.hpp>
#include <vision/vision_structs.hpp>

namespace autoshinyhunthgss {
namespace vision {

// Inclusive hue range plus saturation/value floors, in OpenCV HSV units (H 0-179, S/V 0-255)
// Higher saturation = more vivid; lower saturation = colorless (grey or white)
// Higher value = bright; lower value = black.
struct HsvBand
{
    int h_lo = 0;
    int h_hi = 0;
    int s_min = 0;
    int v_min = 0;
};

// HGSS single-battle layout. Same for every wild encounter, so any hunt method can reuse it.
// Measurements based on test_data.
struct SingleBattleUiConfig
{
    // A full HP bar is one unbroken green run of ~90 px. Windows are padded for crop drift
    // (bars seen up to ~12 px off nominal).
    cv::Rect enemy_bar_window{90, 70, 120, 36};
    cv::Rect player_bar_window{375, 218, 135, 40};
    HsvBand hp_green{40, 85, 70, 60};
    int bar_gap_tolerance = 2; // px of non-green allowed inside a run (camera noise)
    int min_bar_run = 70;

    // Exposure gate
    cv::Rect text_box{40, 300, 380, 60};
    double min_text_box_saturation = 35.0;
};

// Per-species color profile: where the sprite sits and which hues separate normal from shiny.
struct TargetProfile
{
    cv::Rect sprite_roi;
    int sprite_s_min = 0; // only pixels at or above these floors are counted
    int sprite_v_min = 0;
    HsvBand normal_band; // hue that dominates the normal sprite
    HsvBand shiny_band; // hue that dominates the shiny sprite
    double shiny_max_ratio = 0.0; // normal/shiny pixel ratio at or below this -> Shiny
    double shiny_min_fraction = 0.0; // shiny-band fraction of ROI at or above this -> Shiny
    double normal_min_ratio = 0.0; // NotShiny needs ratio at or above this
    double normal_max_fraction = 0.0; // Shiny-band fraction at or below this
};

// Returns the calibrated profile for a target. Lives in vision so the numbers stay next to the
// algorithm that interprets them.
std::optional<TargetProfile> target_profile(HuntTarget target);

} // namespace vision
} // namespace autoshinyhunthgss

#endif // VISION_CONFIG_HPP