// tests/screen_state_test.cpp

#include <gtest/gtest.h>

#include <vision/vision_enums.hpp>

using namespace autoshinyhunthgss::vision;

TEST(ScreenStateTest, InvalidScreenConvertToString)
{
    const auto invalidScreenState = static_cast<ScreenState>(-1);
    EXPECT_EQ(to_string(invalidScreenState), "Unknown");
}

TEST(ScreenStateTest, EncounterShinyConvertToString)
{
    EXPECT_EQ(to_string(ScreenState::EncounterShiny), "EncounterShiny");
}