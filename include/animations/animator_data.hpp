#pragma once

#include <optional>
#include <string>
#include <vector>

struct AnimationParameters;

struct AnimationWhen
{
    std::optional<bool> alive;
    std::optional<bool> knockback;
    std::optional<bool> swinging;
    std::optional<bool> dashing;
    std::optional<bool> onGround;
    std::optional<bool> climbing;
    std::optional<bool> onWall;
    std::optional<bool> rising;
    std::optional<bool> falling;
    std::optional<bool> moving;
    std::optional<bool> finished;
    std::optional<std::string> inState;

    bool operator==(const AnimationWhen &) const = default;
};

struct AnimationTransitionData
{
    std::string from;
    std::string to;
    AnimationWhen when;

    bool operator==(const AnimationTransitionData &) const = default;
};

struct AnimatorData
{
    std::vector<AnimationTransitionData> transitions;

    bool operator==(const AnimatorData &) const = default;
};

bool holds(const AnimationWhen &when, const AnimationParameters &parameters);
