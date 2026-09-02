#include <array>
#include <cfloat>
#include <cstddef>
#include <string>
#include <imgui.h>
#include <string_view>
#include <optional>
#include "ui/editor_ui.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/palette_renamed.hpp"
#include "ui/tile_palettes_ui.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include "ui/editor_section.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "ui/save_controls.hpp"

namespace
{

    std::string_view nameOf(EditorSection section)
    {
        for (const auto &[listed, name] : EditorSections)
            if (listed == section)
                return name;

        return {};
    }
}

void EditorUi::draw(
    const ImGuiManager &imGuiManager,
    const EditorSubject &subject,
    bool showEditors)
{
    if (!showEditors)
        return;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - InspectorWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(InspectorWidth, displaySize.y), ImGuiCond_Always);

    if (!ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    std::array<SectionSaving, EditorSections.size()> saving;
    std::array<bool, EditorSections.size()> unsaved{};
    for (std::size_t at = 0; at < EditorSections.size(); ++at)
    {
        saving[at] = savingIn(EditorSections[at].first, subject);
        unsaved[at] = saving[at].unsaved;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) the names are string literals
    if (ImGui::BeginCombo("##section", nameOf(section).data()))
    {
        for (std::size_t at = 0; at < EditorSections.size(); ++at)
        {
            const auto &[listed, name] = EditorSections[at];
            if (unsaved[at])
                ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);

            // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) a string literal again
            if (ImGui::Selectable(name.data(), listed == section))
                section = listed;

            if (unsaved[at])
                ImGui::PopStyleColor();
        }

        ImGui::EndCombo();
    }

    drawSaveRow(saving);

    ImGui::Separator();

    switch (section)
    {
    case EditorSection::Playback:
        playbackUi.draw(subject.paused, commands);
        break;

    case EditorSection::Game:
        gameSettingsUi.draw(subject.gameData, commands);
        break;

    case EditorSection::Camera:
        cameraUi.draw(subject.gameData, subject.camera, commands);
        break;

    case EditorSection::Player:
        playerUi.draw(subject.gameData, commands);
        break;

    case EditorSection::NpcTypes:
        npcTypesUi.draw(subject.gameData);
        break;

    case EditorSection::Levels:
        levelsUi.draw(
            subject.levels, subject.level, commands, levelUi.hasUnsavedChanges(subject.level));
        break;

    case EditorSection::Level:
        levelUi.draw(
            subject.level,
            subject.npcs,
            subject.playerMotionState,
            subject.playerFeet,
            subject.playerState,
            subject.textures.get(subject.level.getTileMap().getTileSet().texture),
            subject.gameData.tilePalettes.at(subject.level.getTileMap().getTilePalette()),
            subject.gameData.npcData,
            armed,
            commands);
        break;

    case EditorSection::TilePalettes: {
        std::optional<PaletteRenamed> renamed =
            tilePalettesUi.draw(subject.gameData.tilePalettes, subject.textures, commands, armed);
        if (renamed && subject.level.getTileMap().getTilePalette() == renamed->from)
            subject.level.getTileMap().setTilePalette(renamed->to);
    }
    break;
    }

    ImGui::End();
}

void EditorUi::drawOverlays(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    const Player &player)
{
    levelUi.drawOverlay(imGuiManager, camera, level);
    playerUi.drawOverlay(imGuiManager, camera, player);
}

void EditorUi::update(
    float deltaTime,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level)
{
    playerUi.update(deltaTime);
    MouseOnTheMap mouse{
        imGuiManager.getIO().WantCaptureMouse,
        imGuiManager.screenToWorld(
            ImGui::GetMousePos(), camera.getZoom(), camera.getTopLeftPosition()),
        ImGui::IsMouseDown(ImGuiMouseButton_Left),
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)};

    levelUi.update(mouse, level, armed, commands);
}

void EditorUi::drawSaveRow(const std::array<SectionSaving, EditorSections.size()> &saving)
{
    std::optional<std::string> blocked;
    bool anyUnsaved = false;
    for (const SectionSaving &listed : saving)
        anyUnsaved = anyUnsaved || listed.unsaved;

    if (!anyUnsaved)
        return;

    ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
    ImGui::TextUnformatted("unsaved");
    ImGui::PopStyleColor();

    float rightEdge = ImGui::GetCursorScreenPos().x + ImGui::GetContentRegionAvail().x;
    bool first = true;
    for (std::size_t at = 0; at < EditorSections.size(); ++at)
    {
        if (!saving[at].unsaved)
            continue;

        const auto &[listed, name] = EditorSections[at];
        std::string label = "save " + std::string(name);
        if (!first && ImGui::GetCursorScreenPos().x + ImGui::CalcTextSize(label.c_str()).x +
                              ImGui::GetStyle().FramePadding.x * 2.0f <
                          rightEdge)
            ImGui::SameLine();

        first = false;
        ImGui::BeginDisabled(saving[at].cannotBecause.has_value());
        if (ImGui::SmallButton(label.c_str()) && saving[at].save)
            saving[at].save();

        ImGui::EndDisabled();
        if (saving[at].cannotBecause)
            blocked = saving[at].cannotBecause;
    }

    if (blocked)
        ImGui::TextColored(CannotSaveColour, "%s", blocked->c_str());
}

SectionSaving EditorUi::savingIn(EditorSection listed, const EditorSubject &subject)
{
    switch (listed)
    {
    case EditorSection::Game:
        return {
            gameSettingsUi.hasUnsavedChanges(subject.gameData),
            std::nullopt,
            [this, &subject] { gameSettingsUi.save(subject.gameData); }};

    case EditorSection::Camera:
        return {
            cameraUi.hasUnsavedChanges(subject.gameData),
            std::nullopt,
            [this, &subject] { cameraUi.save(subject.gameData); }};

    case EditorSection::Player:
        return {
            playerUi.hasUnsavedChanges(subject.gameData),
            std::nullopt,
            [this, &subject] { playerUi.save(subject.gameData); }};

    case EditorSection::Levels:
        return {
            levelsUi.hasUnsavedChanges(subject.levels),
            std::nullopt,
            [this, &subject] { levelsUi.save(subject.levels); }};

    case EditorSection::Level:
        return {
            levelUi.hasUnsavedChanges(subject.level),
            npcsThatCannotGetBack(subject.level),
            [this, &subject] { levelUi.save(subject.level); }};

    case EditorSection::NpcTypes:
        return {
            npcTypesUi.hasUnsavedChanges(subject.gameData),
            std::nullopt,
            [this, &subject] { npcTypesUi.save(subject.gameData); }};

    case EditorSection::TilePalettes:
        return {
            tilePalettesUi.hasUnsavedChanges(subject.gameData.tilePalettes),
            std::nullopt,
            [this, &subject] { tilePalettesUi.save(subject.gameData.tilePalettes); }};

    case EditorSection::Playback:
        break;
    }

    return {};
}

void EditorUi::valuesReplaced()
{
    gameSettingsUi.valuesReplaced();
    cameraUi.valuesReplaced();
    playerUi.valuesReplaced();
    npcTypesUi.valuesReplaced();
    tilePalettesUi.valuesReplaced();
    levelUi.valuesReplaced();
    levelsUi.valuesReplaced();
}
