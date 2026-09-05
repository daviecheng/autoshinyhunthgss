#ifndef VISION_ENUMS_HPP
#define VISION_ENUMS_HPP

#include <cstdint>
#include <string_view>

namespace autoshinyhunthgss {
namespace vision {

enum class ScreenState : std::uint8_t {
    Unknown,
    GameIntro,
    UserProfileSelect,
    OverworldIdle,
    Encounter,
    EncounterNotShiny,
    EncounterShiny,
    Count
};

std::string_view to_string(ScreenState state)
{
    switch (state) {
        case ScreenState::GameIntro:
            return "GameIntro";
        case ScreenState::UserProfileSelect:
            return "UserProfileSelect";
        case ScreenState::OverworldIdle:
            return "OverworldIdle";
        case ScreenState::Encounter:
            return "Encounter";
        case ScreenState::EncounterNotShiny:
            return "EncounterNotShiny";
        case ScreenState::EncounterShiny:
            return "EncounterShiny";
        case ScreenState::Unknown:
        default:
            return "Unknown";
    }
}

} // namespace vision
} // namespace autoshinyhunthgss

#endif //VISION_ENUMS_HPP