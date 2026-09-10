#include <cfloat>
#include <cstddef>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "ui/when_field.hpp"
#include "ui/data_inspector.hpp"
#include "ui/facts_in_scope.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "animations/animator_facts.hpp"
#include "actor/actor_behavior_context.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "conditions/asked.hpp"
#include "conditions/fact_rows.hpp"
#include "conditions/facts.hpp"

namespace when_field
{
    inspector::Edited drawAsked(std::string_view name, AskedKind kind, Asked &asked)
    {
        switch (kind)
        {
        case AskedKind::YesOrNo:
            return inspector::drawNamed(name, std::get<bool>(asked));
        case AskedKind::Number:
            return inspector::drawNamed(name, std::get<float>(asked));
        case AskedKind::Name:
            return inspector::drawNamed(name, std::get<std::string>(asked));
        }

        return {};
    }

    bool drawAddingAFact(
        std::map<std::string, Asked> &when,
        std::span<const std::string_view> names,
        std::span<const AskedKind> kinds)
    {
        ImGui::BeginDisabled(names.empty());
        if (ImGui::SmallButton("+"))
            ImGui::OpenPopup("##askAbout");
        ImGui::EndDisabled();

        bool added = false;
        if (ImGui::BeginPopup("##askAbout"))
        {
            for (std::size_t index = 0; index < names.size(); ++index)
            {
                std::string label(names[index]);
                if (!ImGui::Selectable(label.c_str()))
                    continue;

                when.emplace(label, emptyOf(kinds[index]));
                added = true;
            }

            ImGui::EndPopup();
        }

        return added;
    }
}

std::vector<FactOffered> factsOffered(const Facts *declared)
{
    std::vector<FactOffered> offered;
    for (const FactRow<ActorBehaviorContext> &row : behaviorRows())
        offered.push_back({std::string(row.name), row.kind});

    if (declared)
        for (const auto &[name, value] : *declared)
            offered.push_back({name, kindOf(value)});

    return offered;
}

inspector::Edited drawCustomField(std::string_view name, AnimationWhen &value)
{
    return drawWhen(name, value, animatorRows());
}

inspector::Edited drawCustomField(std::string_view name, BehaviorWhen &value)
{
    return drawWhen(name, value, factsOffered(factsInScope()));
}
