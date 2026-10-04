#ifndef CAPTURE_CONSTANTS_P_HPP
#define CAPTURE_CONSTANTS_P_HPP

namespace autoshinyhunthgss {
namespace capture {

// The DS top screen is 256x192. Perspective from an off-axis mount stretches the
// measured ratio, so the accepted band is wide; the edge evidence does the real
// discriminating.
constexpr double kMinScreenAspectRatio = 1.10;
constexpr double kMaxScreenAspectRatio = 1.65;

// Fraction of the frame a candidate may occupy. The screen fills roughly a
// quarter to a half of a correctly mounted shot.
constexpr double kMinScreenAreaRatio = 0.08;
constexpr double kMaxScreenAreaRatio = 0.75;

// Each side must span at least this fraction of the frame, so a candidate is
// never built from two nearly coincident lines.
constexpr double kMinScreenSideRatio = 0.20;

// Edges are searched on a downscaled copy. Enough resolution for the screen's
// dark rim to stay a few pixels wide, small enough for the Pi.
constexpr double kWorkingWidth = 800.0;

// Line segments shorter than this fraction of the working width are text, sprites
// or noise rather than a screen border.
constexpr double kMinLineLengthRatio = 0.05;

// Segments within this angle of horizontal (or of vertical) can be a screen side.
// Anything steeper is not a plausible mount.
constexpr double kMaxLineTiltDegrees = 30.0;

// Segments of the same border split into collinear pieces; pieces closer than
// these are merged so they do not crowd out other lines.
constexpr double kDuplicateLineAngleDegrees = 2.0;
constexpr double kDuplicateLineDistance = 2.0;

// Longest lines kept per orientation. Candidates grow with the fourth power of
// this, so it bounds the search.
constexpr int kMaxLinesPerOrientation = 30;

// Rim cue: just inside the screen is the dark unlit rim of the LCD, just outside
// is the brighter bezel. Offsets are in working pixels along the inward normal;
// the inside window starts slightly outward because the strongest line is often
// the lit content's edge a pixel or two inside the rim.
constexpr float kRimInsideOffsets[] = {-2.0F, -1.0F, 0.0F, 1.0F, 2.0F, 3.0F};
constexpr float kRimOutsideOffsets[] = {3.0F, 4.0F, 5.0F, 6.0F, 7.0F};

// The bezel must be this much brighter than the rim. Relative, because a dim room
// leaves the bezel only ~10 grey levels above the rim.
constexpr float kRimContrastRatio = 1.3F;
constexpr float kRimContrastMargin = 4.0F;

// Glow cue: in a dark room the bezel falls below the rim's own brightness, so the
// only border is lit content against darkness.
constexpr float kGlowInsideOffsets[] = {1.0F, 2.0F, 3.0F, 4.0F};
constexpr float kGlowOutsideOffsets[] = {2.0F, 3.0F, 4.0F, 5.0F, 6.0F};
constexpr float kGlowMaxOutside = 60.0F;
constexpr float kGlowContrastRatio = 2.0F;
constexpr float kGlowContrastMargin = 30.0F;

// A corner may sit this fraction of the frame beyond its edge, for a screen that only
// just touches it. Further out the screen is cut off, and a crop would be part guesswork.
constexpr double kMaxCornerOverhangRatio = 0.02;

// Score is half the weakest side's support, half the mean, plus a bonus for size
// so the full screen beats a text box or HP panel inside it.
constexpr double kAreaScoreWeight = 0.3;

// Best score below this is reported as not found. On the rig photos real screens
// scored 0.83 and up, and nothing that was not a screen scored above 0.77.
constexpr double kMinScreenScore = 0.8;

} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_CONSTANTS_P_HPP
