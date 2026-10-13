#ifndef VISION_HEALTH_BAR_DETECTOR_HPP
#define VISION_HEALTH_BAR_DETECTOR_HPP

#include <opencv2/core.hpp>

#include "vision_config.hpp"

namespace autoshinyhunthgss {
namespace vision {

// Which HP bars are on screen. Run lengths are kept so thresholds can be checked against real frames.
struct HealthBars
{
    int enemy_run = 0;
    int player_run = 0;
    bool has_enemy_bar = false;
    bool has_player_bar = false;
};

// Returns the longest horizontal run of set pixels in any single row of a CV_8U mask.
// Gaps of up to gap_tolerance unset pixels are bridged and count toward the run's length.
int longest_run(const cv::Mat& mask, int gap_tolerance);

// Looks for a full green HP bar in each bar window of 512 x 384 HSV frame.
HealthBars detect_health_bars(const cv::Mat& hsv, const SingleBattleUiConfig& ui);

} // namespace vision
} // namespace autoshinyhunthgs

#endif // VISION_HEALTH_BAR_DETECTOR_HPP