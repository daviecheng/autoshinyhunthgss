#ifndef APP_SOFT_RESET_HUNT_HPP
#define APP_SOFT_RESET_HUNT_HPP

#include "app_structs.hpp"

#include <memory>
#include <string>
#include <vector>

#include <capture/i_frame_source.hpp>

namespace autoshinyhunthgss {
namespace app {
namespace soft_reset_hunt {

// Runs the soft reset hunt. Returns a process exit code.
int run(int argc, char** argv);

// Polls source until it ends, the frame count is reached, or the hunt is
// stopped. Takes the interface rather than the options, so the loop can be
// driven by anything that produces frames.
HuntStats run_hunt(capture::IFrameSource& source, const HuntOptions& options);

// Fills out_source and out_hunt from the command line.
// False means the arguments were not understood and usage should be printed.
bool parse_arguments(int argc, char** argv, SourceOptions& out_source, HuntOptions& out_hunt);

// Builds the frame source described by options, or nullptr if it cannot be had.
std::unique_ptr<capture::IFrameSource> make_source(const SourceOptions& options);

// Every image under directory, searched recursively and sorted so a run is reproducible
std::vector<std::string> find_images(const std::string& directory);

void print_usage();
void report(const HuntStats& stats);

} // namespace soft_reset_hunt
} // namespace app
} // namespace autoshinyhunthgss

#endif // APP_SOFT_RESET_HUNT_HPP
