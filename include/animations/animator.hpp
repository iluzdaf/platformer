#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animation_rule_data.hpp"

struct ActorFacts;

class Animator
{
public:
    explicit Animator(const AnimatorData &data);

    void animate(float deltaTime, const ActorFacts &facts);
    const FrameAnimation &playing() const;
    const std::string &state() const;
    std::vector<std::string> takeCues();
    bool finished() const;

private:
    const std::string &shown(const ActorFacts &facts) const;

    std::vector<AnimationRuleData> rules;
    std::string startClip;
    std::string currentState;
    std::unordered_map<std::string, FrameAnimation> animations;
};
