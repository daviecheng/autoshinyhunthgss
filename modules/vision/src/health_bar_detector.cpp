#include "health_bar_detector.hpp"

#include <algorithm>
#include <cstdint>

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

} // namespace vision
} // namespace autoshinyhunthgss
