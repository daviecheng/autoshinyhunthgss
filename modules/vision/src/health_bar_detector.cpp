#include "health_bar_detector.hpp"

#include <algorithm>
#include <cstdint>

#include "band_mask.hpp"

namespace autoshinyhunthgss {
namespace vision {

int longest_run(const cv::Mat& mask, int gap_tolerance)
{
    int longest = 0;

    for (int row = 0; row < mask.rows; row++)
    {
        const auto* pixels = mask.ptr<std::uint8_t>(row);
        int run_start = -1; // no run yet in this row
        int last_set = -1;

        for (int col = 0; col < mask.cols; col++)
        {
            if (pixels[col] == 0)
            {
                continue;
            }

            const bool is_gap_bridged = (run_start >=0) && ((col - last_set - 1) <= gap_tolerance);
            if (!is_gap_bridged)
            {
                run_start = col;
            }

            last_set = col;
            longest = std::max(longest, last_set - run_start + 1);
        }
    }

    return longest;
}

HealthBars detect_health_bars(const cv::Mat& hsv, const SingleBattleUiConfig& ui)
{
    HealthBars bars;

    bars.enemy_run = longest_run(band_mask(hsv(ui.enemy_bar_window), ui.hp_green), ui.bar_gap_tolerance);
    bars.player_run = longest_run(band_mask(hsv(ui.player_bar_window), ui.hp_green), ui.bar_gap_tolerance);
    bars.has_enemy_bar = bars.enemy_run >= ui.min_bar_run;
    bars.has_player_bar = bars.player_run >= ui.min_bar_run;

    return bars;
}

} // namespace vision
} // namespace autoshinyhunthgss
