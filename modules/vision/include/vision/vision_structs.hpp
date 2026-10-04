#ifndef VISION_STRUCTS_HPP
#define VISION_STRUCTS_HPP

#include <cstdint>

namespace autoshinyhunthgss {
namespace vision {

// The Pokemon being hunted. Vision holds the color profile for each. Consumers pick one.
enum class HuntTarget : std::uint8_t
{
    HoOh,
    Count
};

// How cautious the shiny vote is. Hunt policy, so consumer chooses it.
struct VoteSettings
{
    int votes_to_reset = 5; // N consecutive NotShiny -> EncounterNotShiny
    int max_stable_frames = 15; // K stable frames without reaching N -> EncounterUncertain
};

} // namespace vision
} // namespace autoshinyhunthgss

#endif //VISION_STRUCTS_HPP