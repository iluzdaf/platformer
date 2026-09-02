#include <cfloat>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/types_ui.hpp"
#include "ui/type_shown.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/unsaved_colours.hpp"
#include "game/game_data.hpp"

namespace
{
    constexpr float ButtonsWidth = 108.0f;

    std::string labelOf(const TypeShown &showing)
    {
        if (showing.name.empty())
            return "none";

        return (showing.what == TypeShown::What::Npc ? "npc " : "pickup ") + showing.name;
    }

    template <class T>
    void offer(
        const GameData &gameData,
        const std::map<std::string, T> &types,
        TypeShown::What what,
        TypeShown &showing)
    {
        for (const auto &[name, type] : types)
        {
            TypeShown listed{what, name};
            bool cannot = whyATypeCannotBeSaved(gameData, listed).has_value();
            if (cannot)
                ImGui::PushStyleColor(ImGuiCol_Text, CannotSaveColour);

            if (ImGui::Selectable(labelOf(listed).c_str(), showing == listed))
                showing = listed;

            if (cannot)
                ImGui::PopStyleColor();
        }
    }
}

void TypesUi::drawChooser(GameData &gameData)
{
    ImGui::SetNextItemWidth(-ButtonsWidth);
    if (ImGui::BeginCombo("##type", labelOf(showing).c_str()))
    {
        offer(gameData, gameData.npcData, TypeShown::What::Npc, showing);
        offer(gameData, gameData.pickupData, TypeShown::What::Pickup, showing);
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("add"))
        ImGui::OpenPopup("##addType");

    ImGui::SameLine();
    ImGui::BeginDisabled(showing.name.empty());
    if (ImGui::Button("remove", ImVec2(-FLT_MIN, 0.0f)))
    {
        removeTypeFrom(gameData, showing);
        showing = TypeShown{};
    }

    ImGui::EndDisabled();

    if (!ImGui::BeginPopup("##addType"))
        return;

    if (ImGui::Selectable("npc"))
        showing = addTypeTo(gameData, TypeShown::What::Npc);

    if (ImGui::Selectable("pickup"))
        showing = addTypeTo(gameData, TypeShown::What::Pickup);

    ImGui::EndPopup();
}

void TypesUi::draw(GameData &gameData)
{
    drawChooser(gameData);

    ImGui::Separator();

    if (showing.what == TypeShown::What::Npc)
    {
        auto known = gameData.npcData.find(showing.name);
        if (known == gameData.npcData.end())
        {
            ImGui::TextDisabled("pick a type");
            return;
        }

        inspector::drawFields(known->second);
        return;
    }

    auto known = gameData.pickupData.find(showing.name);
    if (known == gameData.pickupData.end())
    {
        ImGui::TextDisabled("pick a type");
        return;
    }

    inspector::drawFields(known->second);
}

void TypesUi::revert(GameData &gameData)
{
    revertTo(saveable, "npcs", gameData.npcData);
    revertTo(saveable, "pickups", gameData.pickupData);
}

void TypesUi::save(GameData &gameData)
{
    if (saveable.unsaved("npcs", asJson(gameData.npcData)))
    {
        saveNpcData(gameData.npcData);
        saveable.saved("npcs", asJson(gameData.npcData));
    }

    if (saveable.unsaved("pickups", asJson(gameData.pickupData)))
    {
        savePickupData(gameData.pickupData);
        saveable.saved("pickups", asJson(gameData.pickupData));
    }
}

bool TypesUi::unsavedSince(const GameData &gameData)
{
    bool npcs = saveable.unsavedSince("npcs", asJson(gameData.npcData));

    return saveable.unsavedSince("pickups", asJson(gameData.pickupData)) || npcs;
}

void TypesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
