#ifndef VISION_ENUMS_HPP
#define VISION_ENUMS_HPP

#include <cstdint>
#include <string_view>

namespace autoshinyhunthgss {
namespace vision {

enum class ScreenState : std::uint8_t {
    Unknown,
    NotEncounter,
    Encounter,
    EncounterUncertain,
    EncounterNotShiny,
    EncounterShiny,
    Count
};

inline std::string_view to_string(ScreenState state)
{
    switch (state) {
        case ScreenState::NotEncounter:
            return "Not Encounter";
        case ScreenState::Encounter:
            return "Encounter";
        case ScreenState::EncounterUncertain:
            return "Encounter Uncertain";
        case ScreenState::EncounterNotShiny:
            return "Encounter Not Shiny";
        case ScreenState::EncounterShiny:
            return "Encounter Shiny";
        case ScreenState::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace vision
} // namespace autoshinyhunthgss

#endif //VISION_ENUMS_HPP