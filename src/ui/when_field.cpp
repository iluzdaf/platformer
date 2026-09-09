#include <cfloat>
#include <cstddef>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/when_field.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "animations/animator_facts.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "conditions/asked.hpp"

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

                switch (kinds[index])
                {
                case AskedKind::YesOrNo:
                    when.emplace(label, true);
                    break;
                case AskedKind::Number:
                    when.emplace(label, 0.0f);
                    break;
                case AskedKind::Name:
                    when.emplace(label, std::string());
                    break;
                }

                added = true;
            }

            ImGui::EndPopup();
        }

        return added;
    }
}

inspector::Edited drawCustomField(std::string_view name, AnimationWhen &value)
{
    return drawWhen(name, value, animatorRows());
}

inspector::Edited drawCustomField(std::string_view name, BehaviorWhen &value)
{
    return drawWhen(name, value, behaviorRows());
}
