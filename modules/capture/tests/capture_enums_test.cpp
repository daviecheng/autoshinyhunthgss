// tests/capture_enums_test.cpp

#include <gtest/gtest.h>

#include <capture/capture_enums.hpp>

using namespace autoshinyhunthgss::capture;

TEST(CaptureStatusTest, InvalidStatusConvertToString)
{
    const auto invalidCaptureStatus = static_cast<CaptureStatus>(-1);
    EXPECT_EQ(to_string(invalidCaptureStatus), "Unknown");
}

TEST(CaptureStatusTest, OkConvertToString)
{
    EXPECT_EQ(to_string(CaptureStatus::Ok), "Ok");
}
