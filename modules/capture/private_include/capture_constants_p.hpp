#ifndef CAPTURE_CONSTANTS_P_HPP
#define CAPTURE_CONSTANTS_P_HPP

namespace autoshinyhunthgss {
namespace capture {

// The DS top screen is 256x192
constexpr double kExpectedAspectRatio = 4.0 / 3.0;
constexpr double kAspectRatioTolerance = 0.25;

// The screen is dark when the DS is off, and bright or vivid when it is on, so
// candidates are collected at several thresholds in each polarity. Multiple
// level rather than one adaptive threshold because glare and backlight shift
// the histogram unpredictably.
constexpr int kDarkThresholds[] = {70, 100, 130};
constexpr int kBrightThresholds[] = {120, 160, 200};
constexpr int kSaturationThresholds[] = {50, 90, 130};

// Opens away speckle, then closes gaps inside the screen region. Too large and
// the open screen merges with a saturated background.
constexpr int kMaskKernelSize = 15;

// Fraction of the frame a candidate may occupy. The screen fills roughly a
// quarter to a half of a correctly mounted shot.
constexpr double kMinScreenAreaRatio = 0.08;
constexpr double kMaxScreenAreaRatio = 0.70;

// A blurred photo will not reduce to four corners at any single epsilon, so it
// is swept until the hull simplifies to a quadrilateral.
constexpr double kPolygonEpsilonMin = 0.01;
constexpr double kPolygonEpsilonMax = 0.15;
constexpr double kPolygonEpsilonStep = 0.005;

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_CONSTANTS_P_HPP
