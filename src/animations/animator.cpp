#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include "animations/animator.hpp"
#include "animations/animation_parameters.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_animations.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

namespace
{
    std::optional<ActorAnimationState> stateNamed(std::string_view name)
    {
        for (const ActorAnimationSlot &slot : ActorAnimationSlots)
            if (slot.name == name)
                return slot.state;

        return std::nullopt;
    }

    ActorAnimationState stateNamedOrRefuse(const std::string &name, const std::string &where)
    {
        std::optional<ActorAnimationState> state = stateNamed(name);
        if (!state)
            throw std::runtime_error(
                "The animator's ladder " + where + " names \"" + name +
                "\", and there is no such animation state");

        return *state;
    }
}

Animator::Animator() : Animator(theUsualLadder())
{
}

Animator::Animator(const AnimatorData &ladder) : data(ladder)
{
    rungs.reserve(data.transitions.size());
    for (const AnimationTransitionData &transition : data.transitions)
        rungs.push_back(
            Rung{
                transition.from.empty() ? ActorAnimationState::Idle
                                        : stateNamedOrRefuse(transition.from, "leaves from"),
                transition.from.empty(),
                stateNamedOrRefuse(transition.to, "goes to"),
                transition.when});
}

ActorAnimationState Animator::wanted(const Decided &decided, const Observed &observed) const
{
    AnimationParameters parameters = parametersFrom(decided, observed, finished());
    for (const Rung &rung : rungs)
    {
        if (!rung.fromAny && rung.from != currentState)
            continue;

        if (holds(rung.when, parameters))
            return rung.to;
    }

    return currentState;
}

void Animator::animate(float deltaTime, const Decided &decided, const Observed &observed)
{
    ActorAnimationState newState = wanted(decided, observed);
    if (!animations.contains(newState))
        newState = ActorAnimationState::Idle;

    if (newState != currentState)
    {
        currentState = newState;
        animations.at(currentState).reset();
    }

    animations.at(currentState).update(deltaTime);
}

const FrameAnimation &Animator::playing() const
{
    return animations.at(currentState);
}

ActorAnimationState Animator::state() const
{
    return currentState;
}

void Animator::add(ActorAnimationState state, const FrameAnimation &animation)
{
    animations.insert_or_assign(state, animation);
}

std::vector<std::string> Animator::takeCues()
{
    auto playingNow = animations.find(currentState);
    return playingNow == animations.end() ? std::vector<std::string>{}
                                          : playingNow->second.takeCues();
}

bool Animator::finished() const
{
    auto playingNow = animations.find(currentState);
    return playingNow != animations.end() && playingNow->second.finished();
}

const AnimatorData &Animator::ladder() const
{
    return data;
}
