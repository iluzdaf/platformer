#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <imgui.h>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/when_field.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"

namespace
{
    constexpr std::array Kinds{AskedKind::YesOrNo, AskedKind::Number, AskedKind::Name};
    constexpr std::array KindLabels{"yes or no", "number", "name"};

    bool drawDeclaring(Facts &facts)
    {
        if (ImGui::SmallButton("+"))
            ImGui::OpenPopup("##declare");

        if (!ImGui::BeginPopup("##declare"))
            return false;

        static std::array<char, 64> asked{};
        ImGui::SetNextItemWidth(120.0f);
        ImGui::InputTextWithHint("##name", "name", asked.data(), asked.size());
        std::string wanted = asked.data();

        bool declared = false;
        ImGui::BeginDisabled(wanted.empty() || facts.contains(wanted));
        for (std::size_t kind = 0; kind < Kinds.size(); ++kind)
        {
            ImGui::SameLine();
            if (!ImGui::Button(KindLabels[kind]))
                continue;

            facts.emplace(wanted, emptyOf(Kinds[kind]));
            asked.fill(0);
            declared = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndDisabled();
        ImGui::EndPopup();
        return declared;
    }
}

inspector::Edited drawCustomField(std::string_view name, Facts &value)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited;
    std::string takeAway;
    for (auto &[fact, held] : value)
    {
        ImGui::PushID(fact.c_str());
        if (ImGui::SmallButton("-"))
            takeAway = fact;

        ImGui::SameLine();
        edited |= when_field::drawAsked(fact, kindOf(held), held);
        ImGui::PopID();
    }

    bool changed = drawDeclaring(value);
    if (!takeAway.empty())
    {
        value.erase(takeAway);
        changed = true;
    }

    if (changed)
        edited |= inspector::Edited{true, true};

    ImGui::TreePop();
    return edited;
}
