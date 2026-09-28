#include "soft_reset_hunt.hpp"
#include "app_constants.hpp"

#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <thread>

#include <capture/frame_source_factory.hpp>

namespace autoshinyhunthgss {
namespace app {
namespace soft_reset_hunt {
    
namespace {

volatile std::sig_atomic_t g_stop_requested = 0;

void handle_signal(int)
{
    g_stop_requested = 1;
}

} // namespace

int run(int argc, char** argv)
{
    SourceOptions source_options;
    source_options.retention.output_directory = kDefaultOutputDirectory;
    HuntOptions hunt_options;

    if (!parse_arguments(argc, argv, source_options, hunt_options))
    {
        print_usage();
        return 1;
    }

    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    const auto source = make_source(source_options);
    if (!source)
    {
        std::cerr << "No frame source available\n";
        return 1;
    }

    const HuntStats stats = run_hunt(*source, hunt_options);
    report(stats);

    return stats.grabbed > 0 && stats.located == 0 ? 1 : 0;
}

HuntStats run_hunt(capture::IFrameSource& source, const HuntOptions& options)
{
    HuntStats stats;
    capture::Frame frame;
    int consecutive_failures = 0;

    while (g_stop_requested == 0 && (options.max_frames == 0 || stats.grabbed < options.max_frames))
    {
        const auto started_at = std::chrono::steady_clock::now();
        const capture::CaptureStatus status = source.capture_frame(frame);

        if (status == capture::CaptureStatus::NoMoreFrames)
        {
            std::cout << "Source exhausted\n";
            break;
        }

        // TODO: const vision::ScreenState state = screen_classifier.classify(frame);
        // TODO: const strategy::ButtonAction action = hunt_strategy.decide(state);
        // TODO: button_driver.press(action);

        const std::int64_t frame_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started_at).count();

        stats.total_frame_ms += frame_ms;
        stats.longest_frame_ms = std::max(stats.longest_frame_ms, frame_ms);

        std::cout << "[" << stats.grabbed << "] " << capture::to_string(status);

        // frame is only filled on success, so its fields are stale otherwise.
        if (status == capture::CaptureStatus::ScreenFound)
        {
            std::cout << "  seq=" << frame.sequence;
            stats.located++;
            consecutive_failures = 0;
        }
        else
        {
            stats.failed++;
            consecutive_failures++;
            stats.longest_stall = std::max(stats.longest_stall, consecutive_failures);
        }

        std::cout << "  " << frame_ms << "ms\n";

        stats.grabbed++;

        if (options.stall_limit > 0 && consecutive_failures >= options.stall_limit)
        {
            std::cerr << "stalled: " << consecutive_failures
                      << " consecutive failures; check the rig\n";
            break;
        }

        // interval 0 means run flat out, so there is no cadence to miss.
        if (options.interval_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(options.interval_ms));
        }
    }

    return stats;
}

bool parse_arguments(int argc, char** argv, SourceOptions& out_source, HuntOptions& out_hunt)
{
    for (int index = 1; index < argc; index++)
    {
        const std::string argument = argv[index];
        const bool has_value = (index + 1) < argc;

        if (argument == "--source" && has_value)
        {
            out_source.kind = argv[++index];
        }
        else if (argument == "--images" && has_value)
        {
            out_source.images_dir = argv[++index];
        }
        else if (argument == "--output-dir" && has_value)
        {
            out_source.retention.output_directory = argv[++index];
        }
        else if (argument == "--retain" && has_value)
        {
            out_source.retention.max_retained_frames = std::atoi(argv[++index]);
        }
        else if (argument == "--retain-all")
        {
            out_source.retention.is_every_frame_retained = true;
        }
        else if (argument == "--interval-ms" && has_value)
        {
            out_hunt.interval_ms = std::atoi(argv[++index]);
        }
        else if (argument == "--count" && has_value)
        {
            out_hunt.max_frames = std::atoi(argv[++index]);
        }
        else if (argument == "--stall-limit" && has_value)
        {
            out_hunt.stall_limit = std::atoi(argv[++index]);
        }
        else
        {
            return false;
        }
    }

    return true;
}

std::unique_ptr<capture::IFrameSource> make_source(const SourceOptions& options)
{
    if (options.kind == "image")
    {
        capture::ImageSourceConfig config;
        config.image_paths = find_images(options.images_dir);
        config.retention = options.retention;

        std::cout << "replaying " << config.image_paths.size() << " images from "
                  << options.images_dir << "\n";

        return capture::make_image_frame_source(config);
    }

    if (options.kind == "camera")
    {
        capture::CameraSourceConfig config;
        config.retention = options.retention;

        return capture::make_camera_frame_source(config);
    }

    std::cerr << "Unknown source '" << options.kind << "'\n";

    return nullptr;
}

std::vector<std::string> find_images(const std::string& directory)
{
    std::vector<std::string> paths;
    std::error_code error;

    for (const auto& entry : std::filesystem::recursive_directory_iterator(directory, error))
    {
        const std::string extension = entry.path().extension().string();
        if (extension == ".png") // only .png is currently supported
        {
            paths.push_back(entry.path().string());
        }
    }

    // Sorted so a run is reproducible; the filesystem gives no useful order.
    std::sort(paths.begin(), paths.end());

    return paths;
}

void print_usage()
{
    std::cout
        << "soft_reset_hunt [options]\n"
        << "  --source image|camera    frame source (default: camera)\n"
        << "  --images <dir>           directory of photos, searched recursively\n"
        << "  --output-dir <dir>       where frames are retained (default: " << kDefaultOutputDirectory << ")\n"
        << "  --retain <n>             frames kept on disk (default: 50)\n"
        << "  --retain-all             keep successful frames too, not just failures\n"
        << "  --interval-ms <n>        poll period, 0 = flat out (default: 200)\n"
        << "  --count <n>              frames to grab, 0 = until stopped (default: 0)\n"
        << "  --stall-limit <n>        stop after n consecutive failures, 0 = never (default: 0)\n";
}

void report(const HuntStats& stats)
{
    const double mean_ms = stats.grabbed > 0
        ? (static_cast<double>(stats.total_frame_ms) / stats.grabbed)
        : 0.0;

    std::cout << "\nframes    " << stats.grabbed
              << "  (" << stats.located << " located, " << stats.failed << " failed)\n"
              << "frame ms  mean " << mean_ms << ", longest " << stats.longest_frame_ms << "\n"
              << "longest stall " << stats.longest_stall << " consecutive failures\n";
}

} // namespace soft_reset_hunt 
} // namespace app
} // namespace autoshinyhunthgss