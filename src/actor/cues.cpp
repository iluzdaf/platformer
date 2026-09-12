#include <string_view>
#include <vector>
#include "actor/abilities/ability_states.hpp"
#include "actor/cues.hpp"
#include "actor/observed.hpp"

std::vector<std::string_view> cuesOf(
    const AbilityStates &states,
    const Observed &observed,
    float fallFromHeightThreshold)
{
    std::vector<std::string_view> cues;
    if (states.dash.emit)
        cues.emplace_back("onDash");
    if (states.swing.emit)
        cues.emplace_back("onAttack");
    if (states.wallJump.emit)
        cues.emplace_back("onWallJump");
    if (states.wallSlide.emit)
        cues.emplace_back("onWallSliding");
    if (observed.fell > fallFromHeightThreshold)
        cues.emplace_back("onFallFromHeight");
    if (!observed.contacts.wasHitCeiling && observed.contacts.hitCeiling)
        cues.emplace_back("onHitCeiling");
    return cues;
}
