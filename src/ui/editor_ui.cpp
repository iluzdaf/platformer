#include <array>
#include "game/level_data.hpp"
#include <string>
#include <cfloat>
#include <cstddef>
#include <imgui.h>
#include <optional>
#include "ui/editor_ui.hpp"
#include "ui/asked_to_undo.hpp"
#include "ui/editor_history.hpp"
#include "ui/saveable.hpp"
#include <glaze/glaze.hpp>
#include <iostream>
#include <utility>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
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
#include "ui/panel_placement.hpp"

void EditorUi::draw(
    const ImGuiManager &imGuiManager,
    const EditorSubject &subject,
    bool showEditors)
{
    if (!showEditors)
        return;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    PanelPlacement placed = panelPinnedRight(displaySize, panelWidth);
    ImGui::SetNextWindowPos(placed.position, ImGuiCond_Always);
    ImGui::SetNextWindowSizeConstraints(placed.smallest, placed.largest);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, displaySize.y), ImGuiCond_FirstUseEver);

    if (!ImGui::Begin("Editor", nullptr, ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    panelWidth = ImGui::GetWindowWidth();

    std::array<SectionSaving, EditorSections.size()> saving;
    for (std::size_t at = 0; at < EditorSections.size(); ++at)
        saving[at] = savingIn(EditorSections[at].first, subject);

    drawSectionTabs(saving);
    drawSaveRow(saving);
    drawUndoRow(subject);

    ImGui::Separator();

    switch (section)
    {
    case EditorSection::Runtime:
        if (ImGui::CollapsingHeader("Playback", ImGuiTreeNodeFlags_DefaultOpen))
            playbackUi.draw(subject.paused, commands);
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
            cameraUi.draw(subject.gameData, subject.camera, commands);
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_DefaultOpen))
        {
            levelUi.drawOverlayToggles();
            playerOverlayUi.drawToggles();
        }
        break;

    case EditorSection::Game:
        gameSettingsUi.draw(subject.gameData, subject.textures, commands);
        break;

    case EditorSection::Cast:
        typesUi.draw(subject.gameData, subject.textures, commands, &subject.level);
        break;

    case EditorSection::Level:
        if (ImGui::CollapsingHeader("Levels", ImGuiTreeNodeFlags_DefaultOpen))
        {
            levelsUi.draw(
                subject.levels,
                subject.levelPath,
                commands,
                levelUi.unsavedSince(subject.levelData, subject.levelPath));
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Level", ImGuiTreeNodeFlags_DefaultOpen))
        {
            levelUi.draw(
                subject.level,
                subject.levelData,
                subject.gameData.playerData.actorData.animationData,
                subject.playerObserved,
                subject.playerFeet,
                subject.playerState,
                subject.gameData.npcData,
                armed,
                commands);
        }
        ImGui::Separator();
        if (ImGui::CollapsingHeader("Tile palettes"))
            tilePalettesUi.draw(subject.gameData.tilePalettes, subject.textures, commands, armed);
        break;
    }

    forgetsIfNamesChanged(subject);
    remembersWhatChanged(subject, ImGui::IsAnyItemActive());

    ImGui::End();
}

void EditorUi::drawSectionTabs(const std::array<SectionSaving, EditorSections.size()> &saving)
{
    if (!ImGui::BeginTabBar("##sections"))
        return;

    for (std::size_t at = 0; at < EditorSections.size(); ++at)
    {
        const auto &[listed, name] = EditorSections[at];
        std::optional<ImVec4> mark =
            markFor(saving[at].unsaved, saving[at].cannotBecause.has_value());
        if (mark)
            ImGui::PushStyleColor(ImGuiCol_Text, *mark);

        ImGuiTabItemFlags flags = askedToShow && listed == section ? ImGuiTabItemFlags_SetSelected
                                                                   : ImGuiTabItemFlags_None;
        // NOLINTNEXTLINE(bugprone-suspicious-stringview-data-usage) the names are string literals
        if (ImGui::BeginTabItem(name.data(), nullptr, flags))
        {
            section = listed;
            ImGui::EndTabItem();
        }

        if (mark)
            ImGui::PopStyleColor();
    }

    ImGui::EndTabBar();
    askedToShow = false;
}

void EditorUi::drawUndoRow(const EditorSubject &subject)
{
    ImGui::BeginDisabled(!history.anythingToUndo());
    bool pressed = ImGui::Button("undo");
    ImGui::EndDisabled();

    ImGui::SameLine();
    ImGui::TextDisabled("ctrl+z");

    if (pressed || askedToUndo())
        undo(subject);
}

void EditorUi::show(EditorSection listed)
{
    section = listed;
    askedToShow = true;
}

EditorSection EditorUi::shown() const
{
    return section;
}

void EditorUi::remembersWhatChanged(const EditorSubject &subject, bool stillBeingEdited)
{
    std::optional<std::string> was = editing.settled(asJson(subject.gameData), stillBeingEdited);
    if (!was)
        return;

    history.remembers(EditorStep{section, std::move(was), std::nullopt, glm::vec2(0.0f)});
}

void EditorUi::forgetsIfNamesChanged(const EditorSubject &subject)
{
    bool types = typesUi.namesChanged();
    bool palettes = tilePalettesUi.namesChanged();
    if (!types && !palettes)
        return;

    history.forgets();
    editing.startsAgainFrom(asJson(subject.gameData));
}

bool EditorUi::undo(const EditorSubject &subject)
{
    std::optional<EditorStep> back = history.stepBack();
    if (!back)
        return false;

    if (back->gameData)
        putsBack(*back->gameData, subject.gameData);

    if (back->levelData)
    {
        if (back->movingThePlayerBack == glm::vec2(0.0f))
            commands.onLevelEdited(*back->levelData);
        else
            commands.onLevelResized(*back->levelData, back->movingThePlayerBack);
    }

    section = back->section;
    editing.startsAgainFrom(asJson(subject.gameData));

    return true;
}

bool EditorUi::anythingToUndo() const
{
    return history.anythingToUndo();
}

namespace
{
    std::string castOf(const GameData &gameData)
    {
        return asJson(gameData.playerData) + asJson(gameData.npcData) + asJson(gameData.pickupData);
    }
}

void EditorUi::putsBack(const std::string &gameDataAsItWas, GameData &gameData)
{
    GameData asItWas;
    if (glz::read_json(asItWas, gameDataAsItWas))
    {
        std::cerr << "could not put the game data back\n";
        return;
    }

    bool settings = asJson(asItWas.settings) != asJson(gameData.settings);
    bool camera = asJson(asItWas.cameraData) != asJson(gameData.cameraData);
    bool cast = castOf(asItWas) != castOf(gameData);
    bool palettes = asJson(asItWas.tilePalettes) != asJson(gameData.tilePalettes);

    gameData = std::move(asItWas);

    if (settings)
        commands.onSettingsChanged();
    if (camera)
        commands.onCameraChanged();
    if (cast)
        commands.onCastChanged();
    if (palettes)
        commands.onPalettesChanged();
}

void EditorUi::drawOverlays(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    const Player &player)
{
    levelUi.drawOverlay(imGuiManager, camera, level);
    playerOverlayUi.draw(imGuiManager, camera, player);
}

void EditorUi::update(
    float deltaTime,
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    const LevelData &levelData,
    const std::string &levelPath)
{
    playerOverlayUi.update(deltaTime);
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

namespace
{
    template <class Section>
    concept Gated = requires { &Section::cannotSaveBecause; };

    static_assert(Gated<GameSettingsUi>);
    static_assert(Gated<CameraUi>);
    static_assert(Gated<TypesUi>);
    static_assert(Gated<LevelUi>);
    static_assert(Gated<TilePalettesUi>);
    static_assert(Gated<LevelsUi>);

    template <class... Reasons> std::optional<std::string> firstOf(Reasons &&...reasons)
    {
        std::optional<std::string> found;
        ((found = found ? found : reasons), ...);
        return found;
    }
}

SectionSaving EditorUi::savingIn(EditorSection listed, const EditorSubject &subject)
{
    switch (listed)
    {
    case EditorSection::Game:
        return {
            gameSettingsUi.unsavedSince(subject.gameData),
            gameSettingsUi.cannotSaveBecause(subject.gameData),
            [this, &subject] { gameSettingsUi.save(subject.gameData); },
            [this, &subject]
            {
                gameSettingsUi.revert(subject.gameData);
                commands.onSettingsChanged();
            }};

    case EditorSection::Runtime:
        return {
            cameraUi.unsavedSince(subject.gameData),
            cameraUi.cannotSaveBecause(subject.gameData),
            [this, &subject] { cameraUi.save(subject.gameData); },
            [this, &subject]
            {
                cameraUi.revert(subject.gameData);
                commands.onCameraChanged();
            }};

    case EditorSection::Cast:
        return {
            typesUi.unsavedSince(subject.gameData),
            typesUi.cannotSaveBecause(subject.gameData),
            [this, &subject]
            {
                LevelData playing = subject.levelData;
                if (typesUi.save(subject.gameData, playing))
                {
                    levelUi.forgets();
                    commands.onLevelEdited(playing);
                }
            },
            [this, &subject] { typesUi.revert(subject.gameData); }};

    case EditorSection::Level:
        return {
            levelUi.unsavedSince(subject.levelData, subject.levelPath) ||
                tilePalettesUi.unsavedSince(subject.gameData.tilePalettes) ||
                levelsUi.unsavedSince(subject.levels),
            firstOf(
                levelUi.cannotSaveBecause(
                    subject.level, subject.levelData, subject.gameData.npcData),
                tilePalettesUi.cannotSaveBecause(subject.gameData.tilePalettes),
                levelsUi.cannotSaveBecause(subject.levels)),
            [this, &subject]
            {
                levelUi.save(subject.levelData, subject.levelPath);
                LevelData playing = subject.levelData;
                if (tilePalettesUi.save(subject.gameData.tilePalettes, playing))
                {
                    levelUi.forgets();
                    commands.onLevelEdited(playing);
                }
                levelsUi.save(subject.levels);
            },
            [this, &subject]
            {
                levelUi.forgets();
                commands.onLoadLevel(subject.levelPath);
                tilePalettesUi.revert(subject.gameData.tilePalettes);
                levelsUi.revert(subject.levels);
            }};
    }

    return {};
}

EditorUi::Reloaded EditorUi::reloaded(GameData &current, const GameData &onDisk)
{
    if (gameSettingsUi.reloaded(current, onDisk))
        commands.onSettingsChanged();

    if (cameraUi.reloaded(current, onDisk))
        commands.onCameraChanged();

    Reloaded taken;
    taken.cast = typesUi.reloaded(current, onDisk);
    taken.palettes = tilePalettesUi.reloaded(current.tilePalettes, onDisk.tilePalettes);
    levelsUi.reloaded(current.levels, onDisk.levels);
    return taken;
}

bool EditorUi::levelTakesTheDisk(const LevelData &current, const std::string &levelPath)
{
    return levelUi.takesTheDisk(current, levelPath);
}
