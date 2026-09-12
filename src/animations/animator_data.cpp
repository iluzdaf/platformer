#include <algorithm>
#include <optional>
#include <string>
#include "animations/animation_rule_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/animator_facts.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    bool someRuleShows(const AnimatorData &data, const std::string &clip)
    {
        return std::ranges::any_of(
            data.rules, [&](const AnimationRuleData &rule) { return rule.show == clip; });
    }
}

std::optional<std::string> whyNotAnAnimator(const AnimatorData &data)
{
    if (data.clips.empty())
        return "has no clips";

    if (data.startClip.empty())
        return "names no clip to start in";

    if (!clipNamed(data, data.startClip))
        return "starts in \"" + data.startClip + "\", and there is no such clip";

    for (const AnimationRuleData &rule : data.rules)
    {
        if (!clipNamed(data, rule.show))
            return "has a rule showing \"" + rule.show + "\", and there is no such clip";

        if (std::optional<std::string> why = whyNotAsked(rule.when, animatorRows()))
            return "has a rule showing \"" + rule.show + "\" that " + *why;
    }

    for (const auto &[name, clip] : data.clips)
        if (name != data.startClip && !someRuleShows(data, name))
            return "never shows \"" + name + "\": it does not start in it, and no rule shows it";

    return std::nullopt;
}
