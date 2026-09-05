#ifndef SCREEN_CLASSIFIER_HPP
#define SCREEN_CLASSIFIER_HPP

#include <vision/vision_enums.hpp>

namespace autoshinyhunthgss {
namespace vision {

class ScreenClassifier
{
public:
    ScreenState classify(bool is_shiny) const;
};

} // namespace vision
} // namespace autoshinyhunthgss

#endif