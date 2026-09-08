#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include "actor/actor_animation_state.hpp"
#include "actor/actor_animation_data.hpp"
#include "animations/frame_animation_data.hpp"

struct ActorAnimationSlot
{
    ActorAnimationState state;
    const char *name;
    std::optional<FrameAnimationData> ActorAnimationData::*said;
};

inline constexpr std::array ActorAnimationSlots{
    ActorAnimationSlot{ActorAnimationState::Idle, "idle", nullptr},
    ActorAnimationSlot{ActorAnimationState::Walk, "walk", &ActorAnimationData::walk},
    ActorAnimationSlot{ActorAnimationState::Dash, "dash", &ActorAnimationData::dash},
    ActorAnimationSlot{ActorAnimationState::Jump, "jump", &ActorAnimationData::jump},
    ActorAnimationSlot{ActorAnimationState::Fall, "fall", &ActorAnimationData::fall},
    ActorAnimationSlot{ActorAnimationState::WallSlide, "wallSlide", &ActorAnimationData::wallSlide},
    ActorAnimationSlot{ActorAnimationState::Climb, "climb", &ActorAnimationData::climb},
    ActorAnimationSlot{ActorAnimationState::Attack, "attack", &ActorAnimationData::attack},
    ActorAnimationSlot{ActorAnimationState::Knockback, "knockback", &ActorAnimationData::knockback},
    ActorAnimationSlot{ActorAnimationState::Dead, "dead", &ActorAnimationData::dead}};

static_assert(
    ActorAnimationSlots.size() == static_cast<std::size_t>(ActorAnimationState::Count),
    "Every animation state needs a slot, so that naming it and finding its data follow from one "
    "list");

inline const char *toString(ActorAnimationState state)
{
    for (const ActorAnimationSlot &slot : ActorAnimationSlots)
        if (slot.state == state)
            return slot.name;

    return "Unknown";
}

inline const FrameAnimationData *saidFor(
    const ActorAnimationData &animations,
    const ActorAnimationSlot &slot)
{
    if (slot.said == nullptr)
        return &animations.idle;

    const std::optional<FrameAnimationData> &said = animations.*slot.said;
    return said ? &said.value() : nullptr;
}
