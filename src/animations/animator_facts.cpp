#include <array>
#include <span>
#include <string>
#include <vector>
#include "animations/animator_facts.hpp"
#include "actor/actor_fact_rows.hpp"
#include "actor/actor_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"

namespace
{
    using Row = FactRow<ActorFacts>;

    constexpr std::array PictureRows{
        Row{"finished",
            AskedKind::YesOrNo,
            "clip finished",
            "clip playing",
            [](const Asked &asked, const ActorFacts &facts)
            { return std::get<bool>(asked) == facts.finished; },
            ""},
        Row{"inState",
            AskedKind::Name,
            "in state",
            "",
            [](const Asked &asked, const ActorFacts &facts)
            { return std::get<std::string>(asked) == facts.inState; },
            ""},
    };
}

std::span<const FactRow<ActorFacts>> animatorRows()
{
    static const std::vector<Row> rows = []
    {
        std::vector<Row> all(actorRows().begin(), actorRows().end());
        all.insert(all.end(), PictureRows.begin(), PictureRows.end());
        return all;
    }();

    return rows;
}
