#include <imgui.h>
#include <format>
#include <string>
#include "rendering/ui/game_editor_ui.hpp"
#include "actor/actor_animation_state.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_section.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"
#include "game/game_data.hpp"
#include "rendering/ui/data_inspector.hpp"

void GameEditorUi::draw(
    EditorSection section,
    GameData &gameData,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &actorState,
    const Camera2D &camera,
    bool paused,
    EditorCommands &commands)
{
    if (section == EditorSection::Playback)
    {
        if (ImGui::Button(paused ? "play" : "pause", ImVec2(60.0f, 0.0f)))
        {
            if (paused)
                commands.onPlay();
            else
                commands.onPause();
        }

        ImGui::SameLine();
        if (ImGui::Button("step", ImVec2(60.0f, 0.0f)))
            commands.onStep();

        ImGui::TextDisabled("%s", paused ? "stopped" : "running");
        return;
    }

    if (section == EditorSection::Game)
    {
        bool reverted = saveable.drawControls("game", gameData.settings, saveGameSettings);
        ImGui::Separator();
        if (inspector::drawFields(gameData.settings).onCommit || reverted)
            commands.onSettingsChanged();
        return;
    }

    if (section == EditorSection::NpcTypes)
    {
        saveable.drawControls("npcs", gameData.npcData, saveNpcData);
        ImGui::Separator();
        inspector::draw("types", gameData.npcData);
        return;
    }

    if (section == EditorSection::TilePalettes)
    {
        saveable.drawControls("palettes", gameData.tilePalettes, saveTilePalettes);
        ImGui::Separator();
        inspector::draw("palettes", gameData.tilePalettes);
        return;
    }

    if (section == EditorSection::Camera)
    {
        ImGui::TextDisabled("%s", camera.shaking() ? "shaking" : "still");

        ImGui::Separator();
        bool reverted = saveable.drawControls("camera", gameData.cameraData, saveCameraData);
        ImGui::Separator();
        if (inspector::drawFields(gameData.cameraData) || reverted)
            commands.onCameraChanged();
        return;
    }

    if (section != EditorSection::Player)
        return;

    ImGui::Checkbox("AABBs", &drawPlayerAABBs);
    ImGui::SameLine();
    if (ImGui::Button("Respawn"))
        commands.onRespawn();

    if (ImGui::BeginTable("Inspector", 2, ImGuiTableFlags_BordersInnerV))
    {
        auto drawRow = [](const char *label, const std::string &value)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(label);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(value.c_str());
        };

        drawRow(
            "Velocity",
            std::format(
                "{:.2f}, {:.2f}", playerMotionState.velocity.x, playerMotionState.velocity.y));
        drawRow("Position", std::format("{:.2f}, {:.2f}", playerPosition.x, playerPosition.y));

        drawRow("On Ground", playerMotionState.contacts.onGround ? "true" : "false");
        drawRow("Facing Left", actorState.facingLeft ? "true" : "false");
        drawRow(
            "Touching Left Wall", playerMotionState.contacts.touchingLeftWall ? "true" : "false");
        drawRow(
            "Touching Right Wall", playerMotionState.contacts.touchingRightWall ? "true" : "false");
        drawRow("Hit Ceiling", playerMotionState.contacts.hitCeiling ? "true" : "false");

        drawRow("Wall Sliding", playerMotionState.wallSlide.active ? "true" : "false");
        drawRow("Wall Jumping", playerMotionState.wallJump.active ? "true" : "false");
        drawRow("Dashing", playerMotionState.dash.active ? "true" : "false");
        drawRow("Hanging", playerMotionState.wallHang.active ? "true" : "false");

        drawRow("Animation", toString(actorState.currentAnimationState));

        ImGui::EndTable();
    }

    ImGui::Separator();
    saveable.drawControls("player", gameData.playerData, savePlayerData);
    ImGui::Separator();
    inspector::drawFields(gameData.playerData);
}

void GameEditorUi::forget()
{
    saveable.forget();
}
bool GameEditorUi::drawsPlayerAABBs() const
{
    return drawPlayerAABBs;
}
