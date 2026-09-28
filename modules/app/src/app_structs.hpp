#ifndef APP_STRUCTS_HPP
#define APP_STRUCTS_HPP

#include <cstdint>
#include <string>

#include <capture/frame_source_factory.hpp>

namespace autoshinyhunthgss {
namespace app {

struct HuntOptions
{
    int interval_ms = 200;  // pause between frames, 0 runs flat out
    int max_frames = 0;     // 0 runs until the source ends or the hunt is stopped
    int stall_limit = 0;    // consecutive failures before giving up, 0 never gives up
};

// What a completed run reports.
// longest_frame_ms is the figure to compare against interval_ms when deciding
// whether the loop can keep up hardware.
struct HuntStats
{
    int grabbed = 0;
    int located = 0;
    int failed = 0;
    int longest_stall = 0; // worst run of consecutive failures
    std::int64_t total_frame_ms = 0;
    std::int64_t longest_frame_ms = 0;
};

// Which frame source to build, and how it retains frames.
struct SourceOptions
{
    std::string kind = "camera";
    std::string images_dir;
    capture::FrameRetentionConfig retention;
};

} // namespace app
} // namespace autoshinyhunthgss

#endif // APP_STRUCTS_HPP
