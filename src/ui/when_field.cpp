#include <cfloat>
#include <cstddef>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/when_field.hpp"
#include "ui/facts_offered_in_scope.hpp"
#include "conditions/when_data.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "conditions/asked.hpp"

namespace when_field
{
    inspector::Edited drawAsked(std::string_view name, AskedKind kind, Asked &asked)
    {
        switch (kind)
        {
        case AskedKind::YesOrNo:
            return inspector::draw(name, std::get<bool>(asked));
        case AskedKind::Number:
            return inspector::draw(name, std::get<float>(asked));
        case AskedKind::Name:
            return inspector::draw(name, std::get<std::string>(asked));
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

inspector::Edited drawCustomField(std::string_view name, WhenData &value)
{
    const FactsOffered *offered = factsOfferedInScope();
    return drawWhen(name, value, offered ? *offered : FactsOffered{});
}
