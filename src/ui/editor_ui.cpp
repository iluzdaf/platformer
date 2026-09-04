#include <array>
#include "game/level_data.hpp"
#include <string>
#include <cfloat>
#include <cstddef>
#include <imgui.h>
#include <string_view>
#include <optional>
#include "ui/editor_ui.hpp"
#include "ui/actors_in_level.hpp"
#include "ui/tile_palettes_ui.hpp"
#include "ui/mouse_on_the_map.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "game/level.hpp"
#include "player/player.hpp"
#include "ui/editor_section.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "ui/unsaved_colours.hpp"
#include "ui/section_mark.hpp"

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
    for (std::size_t at = 0; at < EditorSections.size(); ++at)
        saving[at] = savingIn(EditorSections[at].first, subject);

    ImGui::SetNextItemWidth(-FLT_MIN);
    // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) the names are string literals
    if (ImGui::BeginCombo("##section", nameOf(section).data()))
    {
        for (std::size_t at = 0; at < EditorSections.size(); ++at)
        {
            const auto &[listed, name] = EditorSections[at];
            std::optional<ImVec4> mark =
                markFor(saving[at].unsaved, saving[at].cannotBecause.has_value());
            if (mark)
                ImGui::PushStyleColor(ImGuiCol_Text, *mark);

            // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) a string literal again
            if (ImGui::Selectable(name.data(), listed == section))
                section = listed;

            if (mark)
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
        gameSettingsUi.draw(subject.gameData, subject.textures, commands);
        break;

    case EditorSection::Camera:
        cameraUi.draw(subject.gameData, subject.camera, commands);
        break;

    case EditorSection::Player:
        playerUi.draw(subject.gameData, commands);
        break;

    case EditorSection::Types:
        typesUi.draw(subject.gameData, subject.textures, commands);
        break;

    case EditorSection::Levels:
        levelsUi.draw(
            subject.levels,
            subject.levelPath,
            commands,
            levelUi.unsavedSince(subject.levelData, subject.levelPath));
        break;

    case EditorSection::Level:
        levelUi.draw(
            subject.level,
            subject.levelData,
            subject.levelPath,
            subject.playerMotionState,
            subject.playerFeet,
            subject.playerState,
            subject.gameData.npcData,
            armed,
            commands);
        break;

    case EditorSection::TilePalettes:
        tilePalettesUi.draw(subject.gameData.tilePalettes, subject.textures, commands, armed);
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
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath)
{
    playerUi.update(deltaTime);
    MouseOnTheMap mouse{
        imGuiManager.getIO().WantCaptureMouse,
        imGuiManager.screenToWorld(
            ImGui::GetMousePos(), camera.getZoom(), camera.getTopLeftPosition()),
        ImGui::IsMouseDown(ImGuiMouseButton_Left),
        ImGui::IsMouseClicked(ImGuiMouseButton_Left)};

    levelUi.update(mouse, level, levelData, levelPath, armed, commands);
}

void EditorUi::drawSaveRow(const std::array<SectionSaving, EditorSections.size()> &saving)
{
    bool anyUnsaved = false;
    for (const SectionSaving &listed : saving)
        anyUnsaved = anyUnsaved || listed.unsaved;

    if (!anyUnsaved)
        return;

    ImGui::PushStyleColor(ImGuiCol_Text, UnsavedColour);
    ImGui::TextUnformatted("unsaved");
    ImGui::PopStyleColor();

    for (std::size_t at = 0; at < EditorSections.size(); ++at)
    {
        const SectionSaving &thing = saving[at];
        if (!thing.unsaved)
            continue;

        const auto &[listed, name] = EditorSections[at];
        ImGui::PushID(static_cast<int>(at));

        std::optional<ImVec4> mark = markFor(thing.unsaved, thing.cannotBecause.has_value());
        if (mark)
            ImGui::PushStyleColor(ImGuiCol_Text, *mark);

        // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) the names are string literals
        ImGui::TextUnformatted(name.data());

        if (mark)
            ImGui::PopStyleColor();

        ImGui::SameLine(SaveColumn);
        ImGui::BeginDisabled(thing.cannotBecause.has_value());
        if (ImGui::SmallButton("save") && thing.save)
            thing.save();

        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::SmallButton("revert") && thing.revert)
            thing.revert();

        ImGui::PopID();
    }
}

SectionSaving EditorUi::savingIn(EditorSection listed, const EditorSubject &subject)
{
    switch (listed)
    {
    case EditorSection::Game:
        return {
            gameSettingsUi.unsavedSince(subject.gameData),
            std::nullopt,
            [this, &subject] { gameSettingsUi.save(subject.gameData); },
            [this, &subject]
            {
                gameSettingsUi.revert(subject.gameData);
                commands.onSettingsChanged();
            }};

    case EditorSection::Camera:
        return {
            cameraUi.unsavedSince(subject.gameData),
            std::nullopt,
            [this, &subject] { cameraUi.save(subject.gameData); },
            [this, &subject]
            {
                cameraUi.revert(subject.gameData);
                commands.onCameraChanged();
            }};

    case EditorSection::Player:
        return {
            playerUi.unsavedSince(subject.gameData),
            std::nullopt,
            [this, &subject] { playerUi.save(subject.gameData); },
            [this, &subject] { playerUi.revert(subject.gameData); }};

    case EditorSection::Levels:
        return {
            levelsUi.unsavedSince(subject.levels),
            std::nullopt,
            [this, &subject] { levelsUi.save(subject.levels); },
            [this, &subject] { levelsUi.revert(subject.levels); }};

    case EditorSection::Level:
        return {
            levelUi.unsavedSince(subject.levelData, subject.levelPath),
            npcsThatCannotGetBack(subject.level),
            [this, &subject] { levelUi.save(subject.levelData, subject.levelPath); },
            [this, &subject] { commands.onLoadLevel(subject.levelPath); }};

    case EditorSection::Types:
        return {
            typesUi.unsavedSince(subject.gameData),
            typesUi.cannotSaveBecause(subject.gameData),
            [this, &subject] { typesUi.save(subject.gameData); },
            [this, &subject] { typesUi.revert(subject.gameData); }};

    case EditorSection::TilePalettes:
        return {
            tilePalettesUi.unsavedSince(subject.gameData.tilePalettes),
            tilePalettesUi.cannotSaveBecause(),
            [this, &subject]
            {
                LevelData playing = subject.levelData;
                if (tilePalettesUi.save(subject.gameData.tilePalettes, playing))
                    commands.onLevelEdited(playing);
            },
            [this, &subject] { tilePalettesUi.revert(subject.gameData.tilePalettes); }};

    case EditorSection::Playback:
        break;
    }

    return {};
}

void EditorUi::reloaded(GameData &current, const GameData &onDisk)
{
    gameSettingsUi.reloaded(current, onDisk);
    cameraUi.reloaded(current, onDisk);
    playerUi.reloaded(current, onDisk);
    typesUi.reloaded(current, onDisk);
    tilePalettesUi.reloaded(current.tilePalettes, onDisk.tilePalettes);
    levelsUi.reloaded(current.levels, onDisk.levels);
}

bool EditorUi::levelFollowsTheDisk(const LevelData &current, const std::string &levelPath)
{
    return levelUi.followsTheDisk(current, levelPath);
}
