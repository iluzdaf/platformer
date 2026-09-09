#pragma once

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include "animations/animator_data.hpp"
#include "animations/frame_animation.hpp"

struct Decided;
struct Observed;

class Animator
{
public:
    explicit Animator(const AnimatorData &ladder);

    void animate(
        float deltaTime,
        const Decided &decided,
        const Observed &observed,
        std::string_view inState = {});
    const FrameAnimation &playing() const;
    void add(const std::string &name, const FrameAnimation &anim);
    const std::string &state() const;
    std::vector<std::string> takeCues();
    bool finished() const;
    const AnimatorData &ladder() const;

private:
    const std::string &wanted(
        const Decided &decided,
        const Observed &observed,
        std::string_view inState) const;

    AnimatorData data;
    std::string currentState = "idle";
    std::unordered_map<std::string, FrameAnimation> animations;
};
