#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

#include <capture/capture_constants.hpp>
#include <vision/vision_structs.hpp>

#include "vision_config.hpp"

namespace autoshinyhunthgss::vision {
namespace tests {

const cv::Rect kFrame(0, 0, capture::kFrameWidth, capture::kFrameHeight);

// A ROI that leaves the frame would throw inside OpenCV at runtime. Catches typos while retuning SingleBattleUiConfig.
TEST(VisionConfigTest, BattleUiRegionsLieInsideTheFrame)
{
    const SingleBattleUiConfig config;
    const std::vector<std::pair<std::string, cv::Rect>> roi{
        {"enemy_bar_window", config.enemy_bar_window},
        {"player_bar_window", config.player_bar_window},
        {"text_box", config.text_box}};

    for (const auto& [name, region] : roi)
    {
        // Every battle UI ROI lies fully inside the frame
        EXPECT_EQ(region & kFrame, region) << name << " leaves the frame: " << region;
    }
}

TEST(VisionConfigTest, EveryTargetHasProfile)
{
    for (int index = 0; index < static_cast<int>(HuntTarget::Count); index++)
    {
        const auto profile = target_profile(static_cast<HuntTarget>(index));

        ASSERT_TRUE(profile.has_value()) << "No profile for target " << index;

        // Every target's sprite ROI lies fully inside the frame
        EXPECT_EQ(profile->sprite_roi & kFrame, profile->sprite_roi)
            << "target " << index << " sprite_roi leaves the frame: " << profile->sprite_roi;
    }
}

TEST(VisionConfigTest, InvalidTargetIsRejected)
{
    int invalid_index = static_cast<int>(HuntTarget::Count) + 1;

    EXPECT_FALSE(target_profile(static_cast<HuntTarget>(invalid_index)).has_value());
}

TEST(VisionConfigTest, EveryProfileHasNormalFromShinyDistinction)
{
    for (int index = 0; index < static_cast<int>(HuntTarget::Count); index++)
    {
        const auto profile = target_profile(static_cast<HuntTarget>(index));
        ASSERT_TRUE(profile.has_value());

        EXPECT_GT(profile->shiny_max_ratio, 0.0) << "target " << index;
        EXPECT_GT(profile->normal_min_ratio, profile->shiny_max_ratio) << "target " << index;
        EXPECT_GT(profile->normal_max_fraction, 0.0) << "target " << index;
        EXPECT_GT(profile->shiny_min_fraction, profile->normal_max_fraction) << "target " << index;
    }
}

} // namespace tests
} // namespace autoshinyhunthgss::vision