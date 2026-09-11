#include <string>
#include <string_view>
#include <vector>
#include "animations/animation_ladder_data.hpp"
#include "animations/animator.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/fact_rows.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animator_data.hpp"
#include "actor/decided.hpp"
#include "actor/observed.hpp"

Animator::Animator(const AnimationLadderData &ladder) : data(ladder)
{
}

const std::string &Animator::wanted(
    const Decided &decided,
    const Observed &observed,
    std::string_view inState) const
{
    AnimatorFacts facts{decided, observed, finished(), inState};
    for (const AnimationTransitionData &rung : data.transitions)
    {
        if (!rung.from.empty() && rung.from != currentState)
            continue;

        if (holds(rung.when, animatorRows(), facts))
            return rung.to;
    }

    return currentState;
}

void Animator::animate(
    float deltaTime,
    const Decided &decided,
    const Observed &observed,
    std::string_view inState)
{
    std::string newState = wanted(decided, observed, inState);
    if (!animations.contains(newState))
        newState = std::string(IdleClip);

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

const std::string &Animator::state() const
{
    return currentState;
}

void Animator::add(const std::string &name, const FrameAnimation &animation)
{
    animations.insert_or_assign(name, animation);
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

const AnimationLadderData &Animator::ladder() const
{
    return data;
}
