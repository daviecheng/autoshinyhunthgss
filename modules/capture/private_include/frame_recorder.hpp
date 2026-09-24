#ifndef CAPTURE_FRAME_RECORDER_HPP
#define CAPTURE_FRAME_RECORDER_HPP

#include <string>
#include <opencv2/core.hpp>

#include <capture/frame_source_factory.hpp>

namespace autoshinyhunthgss {
namespace capture {

/*
This class captures the most recent frames on disk. Each record creates one directory
name <xxxxxx>_<reason> holding the input and output frame captured.

The index counts records in this session and never wraps, so directory names never
collide and always sort oldest to newest. Writing index N deletes index (N - max_retained_frames),
which is what bounds the directory.

Slot directories left by a previous session are cleared on construction.
A future improvement is to retain each session frames by having a output directory for each session.
This will require some refactoring to support that.

The recorder does not decide what is worth keeping. The caller calls record() when a frame
is worth looking at, which today means a capture failure. A failure here never stops a hunt.
*/
class FrameRecorder
{
public:
    explicit FrameRecorder(const FrameRetentionConfig& config);

    bool is_enabled() const;

    void record(const cv::Mat& input_frame, const cv::Mat& output_frame, const std::string& reason);

private:
    void clear_slot_directories() const;
    void remove_slot(int index) const;
    void write_image(const std::string& path, const cv::Mat& image);

    FrameRetentionConfig _config;
    int _next_index = 0;
    bool _is_enabled = false;
    bool _has_reported_write_failure = false;
};


} // namespace capture
} // namespace autoshinyhunthgss

#endif // CAPTURE_FRAME_RECORDER_HPP
