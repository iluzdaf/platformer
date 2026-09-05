#include <cfloat>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/types_ui.hpp"
#include "ui/type_shown.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/sheet_preview.hpp"
#include "actor/actor_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"
#include "assets/sheet_data.hpp"
#include "ui/unsaved_colours.hpp"
#include "game/game_data.hpp"
#include "game/level_data.hpp"
#include "ui/renaming.hpp"
#include "ui/level_rewriting.hpp"
#include "ui/renames.hpp"

namespace
{
    constexpr float ButtonsWidth = 108.0f;
    constexpr float PreviewChooserWidth = 100.0f;

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

TypesUi::TypesUi(std::string levelsDirectory, WriteNpcs writeNpcs, WritePickups writePickups)
    : levelsDirectory(std::move(levelsDirectory)), writeNpcs(std::move(writeNpcs)),
      writePickups(std::move(writePickups))
{
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

void TypesUi::drawRename(const GameData &gameData)
{
    bool npc = showing.what == TypeShown::What::Npc;
    Renaming &renaming = npc ? npcRenaming : pickupRenaming;

    if (!renaming.draw(
            npc ? "an npc" : "a pickup",
            showing.name,
            [npc, &renaming, &gameData](const std::string &name)
            {
                bool has =
                    npc ? gameData.npcData.contains(name) : gameData.pickupData.contains(name);
                return has || renaming.somethingIsBecoming(name);
            }))
        return;

    lookAheadAtLevels(
        renaming,
        levelsDirectory,
        [npc](LevelData &levelData, const Renames &renames)
        {
            return npc ? rewriting::typeIn(levelData.npcs, renames)
                       : rewriting::typeIn(levelData.pickups, renames);
        });
}

void TypesUi::drawShown(GameData &gameData, const TextureCache &textures, EditorCommands &commands)
{
    SheetData *sheet = sheetOf(gameData, showing);
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

    SheetInScope scope{texture, *sheet};
    ShowingSheet offering(scope);

    if (showing.what == TypeShown::What::Npc)
    {
        NpcData &npc = gameData.npcData.at(showing.name);
        drawActorPreview(scope, npc.actorData.animationData);
        inspector::drawFields(npc);
    }
    else
    {
        PickupData &pickup = gameData.pickupData.at(showing.name);
        drawAnimationPreview(scope, pickup.animationData);
        inspector::drawFields(pickup);
    }
}

void TypesUi::drawActorPreview(const SheetInScope &scope, const ActorAnimationData &animations)
{
    if (!scope.texture)
        return;

    std::vector<NamedAnimation> offered = animationsOf(animations);
    const NamedAnimation &shown = animationNamed(offered, previewing);
    drawAnimationPreview(scope, *shown.animation);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(PreviewChooserWidth);
    if (!ImGui::BeginCombo("##previewing", shown.name))
        return;

    for (const NamedAnimation &animation : offered)
        if (ImGui::Selectable(animation.name, animation.name == shown.name))
            previewing = animation.name;

    ImGui::EndCombo();
}

void TypesUi::draw(GameData &gameData, const TextureCache &textures, EditorCommands &commands)
{
    drawChooser(gameData);

    ImGui::Separator();

    if (!showing.name.empty())
    {
        drawRename(gameData);
        ImGui::Separator();
    }

    drawShown(gameData, textures, commands);
}

void TypesUi::revert(GameData &gameData)
{
    revertTo(saveable, "npcs", gameData.npcData, npcRenaming);
    revertTo(saveable, "pickups", gameData.pickupData, pickupRenaming);
}

bool TypesUi::save(GameData &gameData, LevelData &playing)
{
    Renames npcs = npcRenaming.sinceSaved(), pickups = pickupRenaming.sinceSaved();

    bool npcsWritten = writeRenamesIntoLevels(
        npcRenaming,
        levelsDirectory,
        [](LevelData &levelData, const Renames &renames)
        { return rewriting::typeIn(levelData.npcs, renames); });

    bool pickupsWritten = writeRenamesIntoLevels(
        pickupRenaming,
        levelsDirectory,
        [](LevelData &levelData, const Renames &renames)
        { return rewriting::typeIn(levelData.pickups, renames); });

    if (!npcsWritten || !pickupsWritten)
        return false;

    renamesTakeEffect(npcs, gameData.npcData);
    renamesTakeEffect(pickups, gameData.pickupData);
    showing.name =
        nameAfterRenames(showing.what == TypeShown::What::Npc ? npcs : pickups, showing.name);

    if (saveable.unsaved("npcs", asJson(gameData.npcData)))
    {
        writeNpcs(gameData.npcData);
        saveable.saved("npcs", asJson(gameData.npcData));
    }

    if (saveable.unsaved("pickups", asJson(gameData.pickupData)))
    {
        writePickups(gameData.pickupData);
        saveable.saved("pickups", asJson(gameData.pickupData));
    }

    bool npcsRePointed = rewriting::typeIn(playing.npcs, npcs);
    bool pickupsRePointed = rewriting::typeIn(playing.pickups, pickups);

    return npcsRePointed || pickupsRePointed;
}

bool TypesUi::unsavedSince(const GameData &gameData)
{
    bool npcs = saveable.unsavedSince("npcs", asJson(gameData.npcData));
    bool pickups = saveable.unsavedSince("pickups", asJson(gameData.pickupData));

    return npcs || pickups || npcRenaming.pending() || pickupRenaming.pending();
}

std::optional<std::string> TypesUi::cannotSaveBecause(const GameData &gameData) const
{
    if (std::optional<std::string> noSheet = typesNamingNoSheet(gameData))
        return noSheet;

    if (std::optional<std::string> npcs = npcRenaming.cannotSaveBecause())
        return npcs;

    return pickupRenaming.cannotSaveBecause();
}

void TypesUi::show(const TypeShown &type)
{
    showing = type;
}

void TypesUi::reloaded(GameData &current, const GameData &onDisk)
{
    reload(saveable, "npcs", current.npcData, onDisk.npcData);
    reload(saveable, "pickups", current.pickupData, onDisk.pickupData);
}
