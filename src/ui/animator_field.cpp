#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <imgui.h>
#include "ui/animator_field.hpp"
#include "ui/data_inspector.hpp"
#include "ui/graph_shown.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/marked_label.hpp"
#include "ui/saved_in_scope.hpp"
#include "animations/animation_rule_data.hpp"
#include "animations/animator_data.hpp"

namespace
{
    constexpr ImVec4 ShowingColour{0.5f, 1.0f, 0.6f, 1.0f};

    inspector::Edited drawRules(std::vector<AnimationRuleData> &rules)
    {
        inspector::InField here("rules");
        const bool changed = inspector::changedHere(rules);
        inspector::Marking marking(changed);
        if (!inspector::drawFold("rules", changed))
            return {};

        ImGui::TextDisabled("the first rule that holds is shown, else the start clip");
        inspector::Edited edited;
        std::optional<std::size_t> takeAway;
        std::optional<std::size_t> raise;
        for (std::size_t index = 0; index < rules.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            if (ImGui::SmallButton("-"))
                takeAway = index;

            ImGui::SameLine();
            ImGui::BeginDisabled(index == 0);
            if (ImGui::SmallButton("^"))
                raise = index;

            ImGui::EndDisabled();
            ImGui::SameLine();
            edited |= inspector::draw(std::to_string(index), rules[index]);
            ImGui::PopID();
        }

        bool addAsked = ImGui::SmallButton("+");

        if (takeAway)
            rules.erase(rules.begin() + static_cast<std::ptrdiff_t>(*takeAway));
        else if (raise)
            std::swap(rules[*raise - 1], rules[*raise]);
        else if (addAsked)
            rules.emplace_back();

        if (takeAway || raise || addAsked)
            edited |= inspector::Edited{true, true};

        ImGui::TreePop();
        return edited;
    }

    void drawLine(const std::string &line, bool showing)
    {
        if (showing)
            ImGui::TextColored(ShowingColour, "%s", line.c_str());
        else
            ImGui::TextUnformatted(line.c_str());
    }
}

void drawAnimatorRules(const AnimatorData &animations, const std::string &showing)
{
    for (const AnimationRuleData &rule : animations.rules)
        drawLine(rule.show + " when " + whenOf(rule.when), rule.show == showing);

    drawLine("otherwise " + animations.startClip, animations.startClip == showing);
}

inspector::Edited drawCustomField(std::string_view name, AnimatorData &value)
{
    if (!inspector::drawFold(name))
        return {};

    inspector::Edited edited;
    edited |= inspector::draw("clips", value.clips);
    edited |= drawRules(value.rules);
    edited |= inspector::draw("startClip", value.startClip);

    ImGui::TreePop();
    return edited;
}
