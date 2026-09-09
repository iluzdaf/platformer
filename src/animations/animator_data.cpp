#include <optional>
#include "animations/animator_data.hpp"
#include "animations/animation_parameters.hpp"

namespace
{
    bool agrees(std::optional<bool> asked, bool actual)
    {
        return !asked || *asked == actual;
    }

}

bool holds(const AnimationWhen &when, const AnimationParameters &parameters)
{
    return agrees(when.alive, parameters.alive) && agrees(when.knockback, parameters.knockback) &&
           agrees(when.swinging, parameters.swinging) && agrees(when.dashing, parameters.dashing) &&
           agrees(when.onGround, parameters.onGround) &&
           agrees(when.climbing, parameters.climbing) && agrees(when.onWall, parameters.onWall) &&
           agrees(when.rising, parameters.rising) && agrees(when.falling, parameters.falling) &&
           agrees(when.moving, parameters.moving) && agrees(when.finished, parameters.finished) &&
           (!when.inState || *when.inState == parameters.inState);
}
