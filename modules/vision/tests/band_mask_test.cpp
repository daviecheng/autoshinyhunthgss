#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "band_mask.hpp"

namespace autoshinyhunthgss::vision {
namespace tests {

// Builds a 1x1 image with HSV and runs band_mask on it.
// Returns true if the single mask pixel is 255 (white).
bool is_in_band(int hue, int saturation, int value, const HsvBand& band)
{
    const cv::Mat pixel(1, 1, CV_8UC3, cv::Scalar(hue, saturation, value));

    return band_mask(pixel, band).at<std::uint8_t>(0, 0) == 255;
}

const HsvBand kGreen{40, 85, 70, 60};
const HsvBand kRedWrapping{176, 7, 90, 70};

TEST(BandMaskTest, HueInsideBandIsIncluded)
{
    EXPECT_TRUE(is_in_band(60, 200, 200, kGreen));
}

TEST(BandMaskTest, BandEdgesAreInclusive)
{
    EXPECT_TRUE(is_in_band(40, 200, 200, kGreen));
    EXPECT_TRUE(is_in_band(85, 200, 200, kGreen));
}

TEST(BandMaskTest, HueOutsideBandIsExcluded)
{
    EXPECT_FALSE(is_in_band(39, 200, 200, kGreen));
    EXPECT_FALSE(is_in_band(86, 200, 200, kGreen));
}

TEST(BandMaskTest, PixelsBelowSaturationOrValueFloorAreExcluded)
{
    EXPECT_FALSE(is_in_band(60, 69, 200, kGreen)); // washed out
    EXPECT_FALSE(is_in_band(60, 200, 59, kGreen)); // too dark
}

// Red sits on both ends of the hue circle; a wrapping band must catch both sides and nothing between.
TEST(BandMaskTest, WrappingBandCoversBothEndsOfHue)
{
    EXPECT_TRUE(is_in_band(179, 200, 200, kRedWrapping));
    EXPECT_TRUE(is_in_band(176, 200, 200, kRedWrapping));
    EXPECT_TRUE(is_in_band(0, 200, 200, kRedWrapping));
    EXPECT_TRUE(is_in_band(7, 200, 200, kRedWrapping));

    EXPECT_FALSE(is_in_band(8, 200, 200, kRedWrapping));
    EXPECT_FALSE(is_in_band(90, 200, 200, kRedWrapping));
    EXPECT_FALSE(is_in_band(175, 200, 200, kRedWrapping));
}

TEST(BandMaskTeset, WrappingBandAppliesFloorOnBothSides)
{
    EXPECT_FALSE(is_in_band(178, 89, 200, kRedWrapping));
    EXPECT_FALSE(is_in_band(3, 200, 69, kRedWrapping));
}

} // namespace tests
} // namespace autoshinyhunthgss::vision