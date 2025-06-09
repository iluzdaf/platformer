#include <imgui.h>
#include "rendering/ui/debug_control_ui.hpp"

void DebugControlUi::draw(bool showDebugControls)
{
    if (showDebugControls)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 120));
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
        ImGui::End();
    }
}