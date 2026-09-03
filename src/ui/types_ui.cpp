#include <cfloat>
#include <map>
#include <optional>
#include <string>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/types_ui.hpp"
#include "ui/type_shown.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"
#include "assets/sheet.hpp"
#include "ui/unsaved_colours.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "ui/renaming.hpp"
#include "game/renames.hpp"

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

namespace
{
    template <class T>
    void moveKey(std::map<std::string, T> &types, const std::string &from, const std::string &to)
    {
        auto node = types.extract(from);
        node.key() = to;
        types.insert(std::move(node));
    }
}

std::optional<TypeRenamed> TypesUi::drawRename(GameData &gameData)
{
    bool npc = showing.what == TypeShown::What::Npc;
    Renaming &renaming = npc ? npcRenaming : pickupRenaming;

    std::optional<Renamed> renamed = renaming.draw(
        npc ? "an npc" : "a pickup",
        showing.name,
        [npc, &gameData](const std::string &name)
        { return npc ? gameData.npcData.contains(name) : gameData.pickupData.contains(name); });

    if (!renamed)
        return std::nullopt;

    if (npc)
        moveKey(gameData.npcData, renamed->from, renamed->to);
    else
        moveKey(gameData.pickupData, renamed->from, renamed->to);

    showing.name = renamed->to;

    return TypeRenamed{showing.what, *renamed};
}

void TypesUi::drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands)
{
    Sheet *sheet = sheetOf(gameData, showing);
    if (!sheet)
    {
        ImGui::TextDisabled("pick a type");
        return;
    }

    const Texture2D *texture = textures.find(sheet->texture);
    if (!texture && !sheet->texture.empty() && sheet->texture != askedToWarm)
    {
        askedToWarm = sheet->texture;
        commands.onWarmTexture(sheet->texture);
    }

    ShowingSheet offering(SheetInScope{texture, *sheet, 0});

    if (showing.what == TypeShown::What::Npc)
        inspector::drawFields(gameData.npcData.at(showing.name));
    else
        inspector::drawFields(gameData.pickupData.at(showing.name));
}

std::optional<TypeRenamed> TypesUi::draw(
    GameData &gameData,
    const TextureCache &textures,
    EditorCommands &commands)
{
    drawChooser(gameData);

    ImGui::Separator();

    std::optional<TypeRenamed> renamed;
    if (!showing.name.empty())
    {
        renamed = drawRename(gameData);
        ImGui::Separator();
    }

    drawShown(gameData, textures, commands);

    return renamed;
}

void TypesUi::revert(GameData &gameData)
{
    revertTo(saveable, "npcs", gameData.npcData, npcRenaming);
    revertTo(saveable, "pickups", gameData.pickupData, pickupRenaming);
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

    writeRenamesIntoLevels(
        npcRenaming,
        [](LevelData &levelData, const Renames &renames)
        { return renameTypeIn(levelData.npcs, renames); });

    writeRenamesIntoLevels(
        pickupRenaming,
        [](LevelData &levelData, const Renames &renames)
        { return renameTypeIn(levelData.pickups, renames); });
}

bool TypesUi::unsavedSince(const GameData &gameData)
{
    bool npcs = saveable.unsavedSince("npcs", asJson(gameData.npcData));

    return saveable.unsavedSince("pickups", asJson(gameData.pickupData)) || npcs;
}

void TypesUi::show(const TypeShown &type)
{
    showing = type;
}

void TypesUi::valuesReplaced()
{
    saveable.valuesReplaced();
    npcRenaming.forget();
    pickupRenaming.forget();
}
