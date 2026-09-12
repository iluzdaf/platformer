#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animation_ladder_data.hpp"

struct Decided;
struct Observed;

class Animator
{
public:
    explicit Animator(const AnimatorData &data);

    void animate(
        float deltaTime,
        const Decided &decided,
        const Observed &observed,
        std::string_view inState = {});
    const FrameAnimation &playing() const;
    const std::string &state() const;
    std::vector<std::string> takeCues();
    bool finished() const;

private:
    const std::string &wanted(
        const Decided &decided,
        const Observed &observed,
        std::string_view inState) const;

    AnimationLadderData ladder;
    std::string currentState;
    std::unordered_map<std::string, FrameAnimation> animations;
};
