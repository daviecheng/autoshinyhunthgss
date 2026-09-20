#ifndef CAPTURE_CONSTANTS_P_HPP
#define CAPTURE_CONSTANTS_P_HPP

namespace autoshinyhunthgss {
namespace capture {

// Smallest fraction of the wide frame a candidate may occupy.
// Rejects glare spots and small bright objects
constexpr double kMinScreenAreaRatio = 0.02;

// The DS top screen is 256x192
constexpr double kExpectedAspectRatio = 4.0 / 3.0;
constexpr double kAspectRatioTolerance = 0.25;

// Edge detection. The screen is found by its border rather than its brightness.
// So this works whether the screen is lighter or darker than the surrounding plastic.
constexpr int kBlurKernelSize = 5;
constexpr double kCannyLowThreshold = 50.0;
constexpr double kCannyHighThreshold = 150.0;

// Closes small gaps in the detected border so it forms one closed contour
constexpr int kMorphologyKernelSize = 5;


// Contour simplification strength, as a fraction of perimeter.
// Too low and a slightly ragged edge never reduces to four corners
constexpr double kPolygonEpsilonRatio = 0.02;

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_CONSTANTS_P_HPP
