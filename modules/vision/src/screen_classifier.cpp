#include "screen_classifier.hpp"

namespace autoshinyhunthgss {
namespace vision {

ScreenState ScreenClassifier::classify(bool is_shiny) const
{
    if (is_shiny)
    {
        return ScreenState::EncounterShiny;
    }

    return ScreenState::Unknown;
}

} // namespace vision
} // namespace autoshinyhunthgss