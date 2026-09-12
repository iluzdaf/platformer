#include <cfloat>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>
#include <imgui.h>
#include <tuple>
#include "ui/armed.hpp"
#include "ui/types_ui.hpp"
#include "ui/saved_in_scope.hpp"
#include "ui/type_shown.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/in_scope.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/sheet_preview.hpp"
#include "animations/frame_animation_data.hpp"
#include "actor/actor_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "player/player_data.hpp"
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
#include "ui/state_machine_field.hpp"
#include "ui/state_machine_shown.hpp"
#include "game/level.hpp"
#include "npc/npc.hpp"
#include <memory>
#include <set>

namespace
{
    constexpr float ButtonsWidth = 108.0f;
    constexpr float PreviewChooserWidth = 100.0f;

    std::string labelOf(const TypeShown &showing)
    {
        if (showing.what == TypeShown::What::Player)
            return showing.name;

        if (showing.name.empty())
            return "none";

        return (showing.what == TypeShown::What::Npc ? "npc " : "pickup ") + showing.name;
    }

    void offer(const GameData &gameData, const TypeShown &listed, TypeShown &showing)
    {
        bool cannot = whyATypeCannotBeSaved(gameData, listed).has_value();
        if (cannot)
            ImGui::PushStyleColor(ImGuiCol_Text, CannotSaveColour);

        if (ImGui::Selectable(labelOf(listed).c_str(), showing == listed))
            showing = listed;

        if (cannot)
            ImGui::PopStyleColor();
    }

    std::set<std::string> statesLitBy(const Level *live, const std::string &type)
    {
        std::set<std::string> lit;
        if (!live)
            return lit;

        for (const std::unique_ptr<Npc> &npc : live->getNpcs())
            if (npc->type() == type && npc->alive())
                lit.emplace(npc->stateName());

        return lit;
    }

}

TypesUi::TypesUi(
    std::string levelsDirectory,
    WriteNpcs writeNpcs,
    WritePickups writePickups,
    WritePlayer writePlayer)
    : levelsDirectory(std::move(levelsDirectory)), writeNpcs(std::move(writeNpcs)),
      writePickups(std::move(writePickups)), writePlayer(std::move(writePlayer))
{
}

namespace
{
    void drawPlaceArm(const TypeShown &showing, std::optional<Armed> &armed)
    {
        if (showing.what == TypeShown::What::Player || showing.name.empty())
        {
            ImGui::BeginDisabled();
            std::ignore = ImGui::Button("place");
            ImGui::EndDisabled();

            return;
        }

        PlaceOne placing{
            showing.what == TypeShown::What::Npc ? PlaceOne::What::Npc : PlaceOne::What::Pickup,
            showing.name};
        bool isArmed = armed && *armed == Armed{placing};

        if (isArmed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ArmedColour);
        }

        bool clicked = ImGui::Button("place");

        if (isArmed)
            ImGui::PopStyleColor(3);

        if (clicked)
            armed = isArmed ? std::nullopt : std::optional<Armed>(placing);
    }
}

void TypesUi::drawChooser(GameData &gameData, std::optional<Armed> &armed)
{
    ImGui::SetNextItemWidth(-ButtonsWidth);
    if (ImGui::BeginCombo("##type", labelOf(showing).c_str()))
    {
        for (const TypeShown &listed : offered(gameData))
            offer(gameData, listed, showing);
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    drawPlaceArm(showing, armed);

    ImGui::SameLine();
    if (ImGui::Button("add"))
        ImGui::OpenPopup("##addType");

    ImGui::SameLine();
    ImGui::BeginDisabled(showing.what == TypeShown::What::Player || showing.name.empty());
    if (ImGui::Button("remove", ImVec2(-FLT_MIN, 0.0f)))
        remove(gameData);

    ImGui::EndDisabled();

    if (!ImGui::BeginPopup("##addType"))
        return;

    if (ImGui::Selectable("npc"))
        add(gameData, TypeShown::What::Npc);

    if (ImGui::Selectable("pickup"))
        add(gameData, TypeShown::What::Pickup);

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

    namesTouched = true;
    lookAheadAtLevels(
        renaming,
        levelsDirectory,
        [npc](LevelData &levelData, const Renames &renames)
        {
            return npc ? rewriting::typeIn(levelData.npcs, renames)
                       : rewriting::typeIn(levelData.pickups, renames);
        });
}

void TypesUi::drawShown(
    GameData &gameData,
    const TextureCache &textures,
    EditorCommands &commands,
    const Level *live)
{
    const SheetData *sheet = sheetOf(gameData, showing);
    if (!sheet)
    {
        ImGui::TextDisabled("pick a type");
        return;
    }

    const Texture2D *texture = textures.find(sheet->texture.path);
    if (!texture && !sheet->texture.path.empty() && sheet->texture.path != askedToWarm)
    {
        askedToWarm = sheet->texture.path;
        commands.onWarmTexture(sheet->texture.path);
    }

    SheetInScope scope{texture, *sheet};
    InScope offering(scope);

    inspector::Edited edited;
    switch (showing.what)
    {
    case TypeShown::What::Npc: {
        SavedInScope was(asItWasSaved<std::map<std::string, NpcData>>(saveable.lastSeen("npcs")));
        inspector::InField shown(npcRenaming.shownName(showing.name));
        NpcData &npc = gameData.npcData.at(showing.name);
        drawActorPreview(scope, npc.actorData);
        edited |= inspector::drawFieldsExcept(npc, "stateMachineBehaviorData");
        InScope declared(npc.facts);
        inspector::InField machine("stateMachineBehaviorData");
        edited |= drawStateMachineEditor(
            npc.stateMachineBehaviorData, statesLitBy(live, showing.name), machineShown);
        break;
    }

    case TypeShown::What::Pickup: {
        SavedInScope was(
            asItWasSaved<std::map<std::string, PickupData>>(saveable.lastSeen("pickups")));
        inspector::InField shown(pickupRenaming.shownName(showing.name));
        PickupData &pickup = gameData.pickupData.at(showing.name);
        ImVec2 at = drawAnimationPreview(scope, pickup.animationData);
        glm::vec2 drawn = drawnSizeOf(pickup);
        if (drawn.x > 0.0f)
            drawColliderOver(
                at,
                PreviewSize / drawn.x,
                pickup.colliderOffset,
                pickup.colliderSize.value_or(drawn));
        edited |= inspector::drawFields(pickup);
        break;
    }

    case TypeShown::What::Player: {
        SavedInScope was(asItWasSaved<PlayerData>(saveable.lastSeen("player")));
        drawActorPreview(scope, gameData.playerData.actorData);
        edited |= inspector::drawFields(gameData.playerData);
        break;
    }
    }

    if (edited.onCommit)
        commands.onCastChanged();
}

void TypesUi::drawActorPreview(const SheetInScope &scope, const ActorData &actorData)
{
    static const FrameAnimationData nothingDrawnYet{};
    if (!scope.texture)
        return;

    std::vector<NamedAnimation> offered = actorData.animationData
                                              ? animationsOf(*actorData.animationData)
                                              : std::vector<NamedAnimation>{};
    const NamedAnimation *shown = offered.empty() ? nullptr : &animationNamed(offered, previewing);
    ImVec2 at = drawAnimationPreview(scope, shown ? *shown->animation : nothingDrawnYet);

    glm::vec2 drawn = drawnSizeOf(actorData);
    if (drawn.x > 0.0f)
        drawColliderOver(
            at,
            PreviewSize / drawn.x,
            actorData.physicsBodyData.colliderOffset,
            actorData.physicsBodyData.colliderSize);

    if (!shown)
        return;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(PreviewChooserWidth);
    if (!ImGui::BeginCombo("##previewing", shown->name.c_str()))
        return;

    for (const NamedAnimation &animation : offered)
        if (ImGui::Selectable(animation.name.c_str(), animation.name == shown->name))
            previewing = animation.name;

    ImGui::EndCombo();
}

void TypesUi::draw(
    GameData &gameData,
    const TextureCache &textures,
    EditorCommands &commands,
    std::optional<Armed> &armed,
    const Level *live)
{
    TypeShown wasShowing = showing;
    drawChooser(gameData, armed);
    if (showing != wasShowing)
        machineShown = MachineShown{};

    ImGui::Separator();

    if (showing.what != TypeShown::What::Player && !showing.name.empty())
    {
        drawRename(gameData);
        ImGui::Separator();
    }

    drawShown(gameData, textures, commands, live);
}

void TypesUi::revert(GameData &gameData)
{
    revertTo(saveable, "player", gameData.playerData);
    revertTo(saveable, "npcs", gameData.npcData, npcRenaming);
    revertTo(saveable, "pickups", gameData.pickupData, pickupRenaming);
}

bool TypesUi::save(GameData &gameData, LevelData &playing)
{
    Renames npcs = npcRenaming.sinceSaved(), pickups = pickupRenaming.sinceSaved();
    std::vector<std::string> npcsRemoved = npcRenaming.removed();
    std::vector<std::string> pickupsRemoved = pickupRenaming.removed();

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

    for (const std::string &name : npcsRemoved)
        gameData.npcData.erase(name);
    for (const std::string &name : pickupsRemoved)
        gameData.pickupData.erase(name);

    renamesTakeEffect(npcs, gameData.npcData);
    renamesTakeEffect(pickups, gameData.pickupData);
    showing.name =
        nameAfterRenames(showing.what == TypeShown::What::Npc ? npcs : pickups, showing.name);

    if (saveable.unsaved("player", asJson(gameData.playerData)))
    {
        writePlayer(gameData.playerData);
        saveable.saved("player", asJson(gameData.playerData));
    }

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
    bool player = saveable.unsavedSince("player", asJson(gameData.playerData));
    bool npcs = saveable.unsavedSince("npcs", asJson(gameData.npcData));
    bool pickups = saveable.unsavedSince("pickups", asJson(gameData.pickupData));

    return player || npcs || pickups || npcRenaming.pending() || pickupRenaming.pending();
}

std::optional<std::string> TypesUi::cannotSaveBecause(const GameData &gameData)
{
    std::string cast =
        asJson(gameData.playerData) + asJson(gameData.npcData) + asJson(gameData.pickupData);
    if (std::optional<std::string> cannot =
            castGate.to(std::move(cast), [&] { return aTypeThatCannotBeSaved(gameData); }))
        return cannot;

    if (std::optional<std::string> npcs = npcRenaming.cannotSaveBecause())
        return npcs;

    return pickupRenaming.cannotSaveBecause();
}

std::vector<TypeShown> TypesUi::offered(const GameData &gameData) const
{
    std::vector<TypeShown> listed{thePlayer()};
    for (const auto &[name, npc] : gameData.npcData)
        if (!npcRenaming.gone(name))
            listed.push_back(TypeShown{TypeShown::What::Npc, name});

    for (const auto &[name, pickup] : gameData.pickupData)
        if (!pickupRenaming.gone(name))
            listed.push_back(TypeShown{TypeShown::What::Pickup, name});

    return listed;
}

void TypesUi::show(const TypeShown &type)
{
    showing = type;
    machineShown = MachineShown{};
}

void TypesUi::add(GameData &gameData, TypeShown::What what)
{
    show(addTypeTo(gameData, what));
    (what == TypeShown::What::Npc ? npcRenaming : pickupRenaming).added(showing.name);
    namesTouched = true;
}

void TypesUi::remove(GameData &gameData)
{
    if (showing.what == TypeShown::What::Player || showing.name.empty())
        return;

    bool npc = showing.what == TypeShown::What::Npc;
    Renaming &renaming = npc ? npcRenaming : pickupRenaming;
    if (renaming.remove(showing.name, std::string()))
        lookAheadAtLevels(
            renaming,
            levelsDirectory,
            [npc](LevelData &levelData, const Renames &renames)
            {
                return npc ? rewriting::typeIn(levelData.npcs, renames)
                           : rewriting::typeIn(levelData.pickups, renames);
            });
    else
        removeTypeFrom(gameData, showing);

    namesTouched = true;
    show(thePlayer());
}

bool TypesUi::namesChanged()
{
    return std::exchange(namesTouched, false);
}

bool TypesUi::reloaded(GameData &current, const GameData &onDisk)
{
    bool player = reload(saveable, "player", current.playerData, onDisk.playerData);
    bool npcs = reload(saveable, "npcs", current.npcData, onDisk.npcData);
    bool pickups = reload(saveable, "pickups", current.pickupData, onDisk.pickupData);
    return player || npcs || pickups;
}
