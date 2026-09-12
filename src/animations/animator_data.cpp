#include <algorithm>
#include <optional>
#include <string>
#include "animations/animator_data.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    bool onTheLadder(const AnimationLadderData &ladder, const std::string &clip)
    {
        return std::ranges::any_of(
            ladder.transitions,
            [&](const AnimationTransitionData &rung)
            { return rung.to == clip || rung.from == clip; });
    }
}

std::optional<std::string> whyNotAnAnimator(const AnimatorData &data)
{
    if (data.clips.empty())
        return "has no clips, and that is not an animator";

    if (data.startClip.empty())
        return "has no start clip, and that is not an animator";

    if (!clipNamed(data, data.startClip))
        return "starts with \"" + data.startClip + "\", and there is no such clip";

    for (const auto &[name, clip] : data.clips)
        if (name != data.startClip && !onTheLadder(data.ladder, name))
            return "the clip \"" + name + "\" is neither where it starts nor on the ladder";

    for (const AnimationTransitionData &rung : data.ladder.transitions)
    {
        if (!rung.from.empty() && !clipNamed(data, rung.from))
            return "the ladder leaves from \"" + rung.from + "\", and there is no such clip";

        if (!clipNamed(data, rung.to))
            return "the ladder goes to \"" + rung.to + "\", and there is no such clip";

        if (std::optional<std::string> why = whyNotAsked(rung.when, animatorRows()))
            return "the rung to \"" + rung.to + "\" " + *why;
    }

    return std::nullopt;
}