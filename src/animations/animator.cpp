#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include "animations/animation_rule_data.hpp"
#include "animations/animator.hpp"
#include "animations/animator_facts.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animator_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "conditions/fact_rows.hpp"

Animator::Animator(const AnimatorData &data)
    : rules(data.rules), startClip(data.startClip), currentState(data.startClip)
{
    if (std::optional<std::string> why = whyNotAnAnimator(data))
        throw std::runtime_error("An animator " + *why);

    for (const auto &[name, clip] : data.clips)
        animations.insert_or_assign(name, FrameAnimation(clip));
}

const std::string &Animator::shown(
    const AbilityStates &abilityStates,
    const Observed &observed,
    std::string_view inState) const
{
    AnimatorFacts facts{abilityStates, observed, finished(), inState};
    for (const AnimationRuleData &rule : rules)
        if (holds(rule.when, animatorRows(), facts))
            return rule.show;

    return startClip;
}

void Animator::animate(
    float deltaTime,
    const AbilityStates &abilityStates,
    const Observed &observed,
    std::string_view inState)
{
    std::string newState = shown(abilityStates, observed, inState);

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
