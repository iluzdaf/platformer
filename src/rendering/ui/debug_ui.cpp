#include <imgui.h>
#include <format>
#include "rendering/ui/debug_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "game/player/player_state.hpp"
#include "cameras/camera2d.hpp"

void DebugUi::draw(
    const ImGuiManager &imGuiManager,
    const PlayerState &playerState,
    const Camera2D &camera, 
    bool showDebug)
{
    if (!showDebug)
        return;

    ImGui::SetNextWindowSize(ImVec2(200, ImGui::GetIO().DisplaySize.y));
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

        drawRow("Velocity", std::format("{:.2f}, {:.2f}", playerState.velocity.x, playerState.velocity.y));
        drawRow("Position", std::format("{:.2f}, {:.2f}", playerState.position.x, playerState.position.y));

        drawRow("On Ground", playerState.onGround ? "true" : "false");
        drawRow("Facing Left", playerState.facingLeft ? "true" : "false");
        drawRow("Touching Left Wall", playerState.touchingLeftWall ? "true" : "false");
        drawRow("Touching Right Wall", playerState.touchingRightWall ? "true" : "false");
        drawRow("Hit Ceiling", playerState.hitCeiling ? "true" : "false");

        drawRow("Wall Sliding", playerState.wallSliding ? "true" : "false");
        drawRow("Wall Jumping", playerState.wallJumping ? "true" : "false");
        drawRow("Dashing", playerState.dashing ? "true" : "false");
        drawRow("Jump Count", std::format("{}", playerState.jumpCount));

        drawRow("Camera Shaking", camera.shaking() ? "true" : "false");

        ImGui::EndTable();
    }

    ImGui::End();
}