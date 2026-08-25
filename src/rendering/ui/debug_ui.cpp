#include <imgui.h>
#include <format>
#include "rendering/ui/debug_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "game/actor/actor_motion_state.hpp"
#include "game/actor/actor_state.hpp"
#include "cameras/camera2d.hpp"

void DebugUi::draw(
    const ImGuiManager &imGuiManager,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &actorState,
    const Camera2D &camera,
    bool showDebug)
{
    if (!showDebug)
        return;

    ImGui::SetNextWindowSize(ImVec2(200, imGuiManager.getUiDimensions().y));
    ImGui::Begin("Debug");
    if (ImGui::Button("Step"))
        onStep();
    ImGui::SameLine();
    if (ImGui::Button("Play"))
        onPlay();
    ImGui::SameLine();
    if (ImGui::Button("Respawn"))
        onRespawn();
    ImGui::SameLine();
    if (ImGui::Button("Zoom"))
        onToggleZoom();

    if (ImGui::Button("Tile Info"))
        onToggleDrawTileInfo();
    ImGui::SameLine();
    if (ImGui::Button("Grid"))
        onToggleDrawGrid();
    ImGui::SameLine();
    if (ImGui::Button("Player"))
        onToggleDrawPlayerAABBs();

    if (ImGui::Button("TileMap"))
        onToggleDrawTileMapAABBs();
    ImGui::SameLine();
    if (ImGui::Button("Reload"))
        onGameReload();

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

        drawRow("Velocity", std::format("{:.2f}, {:.2f}", playerMotionState.velocity.x, playerMotionState.velocity.y));
        drawRow("Position", std::format("{:.2f}, {:.2f}", playerPosition.x, playerPosition.y));

        drawRow("On Ground", playerMotionState.contacts.onGround ? "true" : "false");
        drawRow("Facing Left", actorState.facingLeft ? "true" : "false");
        drawRow("Touching Left Wall", playerMotionState.contacts.touchingLeftWall ? "true" : "false");
        drawRow("Touching Right Wall", playerMotionState.contacts.touchingRightWall ? "true" : "false");
        drawRow("Hit Ceiling", playerMotionState.contacts.hitCeiling ? "true" : "false");

        drawRow("Wall Sliding", playerMotionState.wallSlide.active ? "true" : "false");
        drawRow("Wall Jumping", playerMotionState.wallJump.active ? "true" : "false");
        drawRow("Dashing", playerMotionState.dash.active ? "true" : "false");
        drawRow("Climbing", playerMotionState.climb.active ? "true" : "false");

        drawRow("Animation", toString(actorState.currentAnimationState));

        drawRow("Camera Shaking", camera.shaking() ? "true" : "false");

        ImGui::EndTable();
    }

    ImGui::End();
}