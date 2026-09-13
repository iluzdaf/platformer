#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "animations/animation_rule_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "conditions/facts.hpp"

struct AnimatorData
{
    std::map<std::string, FrameAnimationData> clips;
    std::vector<AnimationRuleData> rules;
    std::string startClip;
};

inline const FrameAnimationData *clipNamed(const AnimatorData &animations, std::string_view name)
{
    auto found = animations.clips.find(std::string(name));
    return found == animations.clips.end() ? nullptr : &found->second;
}

std::optional<std::string> whyNotAnAnimator(
    const AnimatorData &data,
    const FactsData &declared = FactsData{},
    const SensesData &senses = SensesData{});