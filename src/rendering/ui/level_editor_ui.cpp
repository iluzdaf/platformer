#include <algorithm>
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/levels.hpp"
#include "cameras/camera2d.hpp"
#include "navigation/navigation_graph.hpp"
#include "npc/npc_spawn_data.hpp"

namespace
{
    std::string levelName(const std::string &levelPath)
    {
        std::string name = levelPath.substr(levelPath.find_last_of("/\\") + 1);
        size_t extension = name.rfind(".json");
        return extension == std::string::npos ? name : name.substr(0, extension);
    }
}

void LevelEditorUi::draw(
    const ImGuiManager &imGuiManager,
    Level &level,
    const Texture2D &tileSet,
    bool showLevelEditor)
{
    if (!showLevelEditor)
        return;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - 200, 0));
    ImGui::SetNextWindowSize(ImVec2(200, displaySize.y));
    ImGui::Begin("Level Editor");

    ImGui::Text(
        "%s w%dxh%dxs%d",
        levelName(level.getPath()).c_str(),
        level.getTileMap().getWidth(),
        level.getTileMap().getHeight(),
        level.getTileMap().getTileSize());

    std::optional<std::string> chosenLevel = drawLevelChooser(level);
    if (chosenLevel)
    {
        editing = false;
        ImGui::End();
        onLoadLevel(*chosenLevel);
        return;
    }

    ImGui::TextUnformatted("next");
    ImGui::SameLine();
    if (ImGui::SmallButton(levelName(level.getNextLevel()).c_str()))
    {
        std::string nextLevel = level.getNextLevel();
        editing = false;
        ImGui::End();
        onLoadLevel(nextLevel);
        return;
    }

    const std::vector<NpcSpawnData> &npcs = level.getNpcs();
    if (ImGui::CollapsingHeader("npcs", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        if (npcs.empty())
            ImGui::TextDisabled("none");
        else
            for (const NpcSpawnData &npc : npcs)
                ImGui::Text("%s %d,%d", npc.type.c_str(), npc.tilePosition.x, npc.tilePosition.y);
        ImGui::Unindent();
    }

    drawGraphs(level);

    ImGui::Separator();

    if (ImGui::Button("Tile Info"))
        onToggleDrawTileInfo();
    ImGui::SameLine();
    if (ImGui::Button("Grid"))
        onToggleDrawGrid();

    if (ImGui::Button("Player"))
        onToggleDrawPlayerAABBs();
    ImGui::SameLine();
    if (ImGui::Button("TileMap"))
        onToggleDrawTileMapAABBs();

    if (ImGui::Button("Respawn"))
        onRespawn();

    ImGui::Separator();

    if (!editing)
    {
        if (ImGui::Button("edit"))
            editing = true;

        ImGui::End();
        return;
    }

    const auto &tiles = level.getTileMap().getTiles();
    int columns = 4;
    int count = 0;
    int tileSize = level.getTileMap().getTileSize();
    ImTextureID imguiTextureID = (ImTextureID)(intptr_t)tileSet.getTextureID();

    for (const auto &[tileIndex, tile] : tiles)
    {
        ImGui::PushID(tileIndex);
        auto [uvStart, uvEnd] = tileSet.getUVRange(tileIndex, tileSize, false);
        int previouslySelectedTileIndex = selectedTileIndex;

        if (previouslySelectedTileIndex == tileIndex)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 0, 255));
        }

        ImVec2 tilePos = ImGui::GetCursorScreenPos();
        if (ImGui::ImageButton(
                "##tile",
                imguiTextureID,
                ImVec2(32, 32),
                ImVec2(uvStart.x, uvStart.y),
                ImVec2(uvEnd.x, uvEnd.y)))
            selectedTileIndex = tileIndex;

        ImGui::GetWindowDrawList()->AddText(
            tilePos,
            IM_COL32(255, 255, 255, 255),
            std::to_string(tileIndex).c_str());

        if (previouslySelectedTileIndex == tileIndex)
            ImGui::PopStyleColor(3);

        if (++count % columns != 0)
            ImGui::SameLine();

        ImGui::PopID();
    }

    bool previouslyEditingPlayerStartTile = editingPlayerStartTile;
    if (previouslyEditingPlayerStartTile)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 100, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 100, 255, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 100, 255, 255));
    }

    ImGui::NewLine();
    if (ImGui::Button("Spawn"))
    {
        editingPlayerStartTile = true;
        selectedTileIndex = 0;
    }

    if (previouslyEditingPlayerStartTile)
        ImGui::PopStyleColor(3);

    ImGui::SameLine();
    if (ImGui::Button("Save"))
    {
        level.save();
        editing = false;
    }

    ImGui::SameLine();
    if (ImGui::Button("Reload"))
    {
        onLoadLevel(level.getPath());
        editing = false;
    }

    ImGui::End();
}

std::optional<std::string> LevelEditorUi::drawLevelChooser(const Level &level)
{
    std::string directory = level.getPath().substr(0, level.getPath().find_last_of("/\\"));
    std::optional<std::string> chosen;

    if (ImGui::BeginCombo("level", levelName(level.getPath()).c_str()))
    {
        for (const std::string &path : levelPathsIn(directory))
        {
            bool current = path == level.getPath();
            if (ImGui::Selectable(levelName(path).c_str(), current) && !current)
                chosen = path;
        }
        ImGui::EndCombo();
    }

    if (ImGui::SmallButton("make first"))
        onSetFirstLevel();

    return chosen;
}

void LevelEditorUi::drawGraphs(const Level &level)
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();

    for (size_t index = 0; index < graphs.size(); ++index)
    {
        const NamedNavigationGraph &named = graphs[index];
        ImGui::PushID(static_cast<int>(index));

        std::string label = named.name + " (" +
                            std::to_string(named.graph.getNodes().size()) + " nodes, " +
                            std::to_string(named.graph.getEdges().size()) + " edges)";

        if (ImGui::CollapsingHeader(label.c_str()))
        {
            selectedGraphIndex = index;
            ImGui::Indent();

            std::vector<int> nodeIds;
            for (const auto &[nodeId, node] : named.graph.getNodes())
                nodeIds.push_back(nodeId);
            std::sort(nodeIds.begin(), nodeIds.end());

            for (int nodeId : nodeIds)
            {
                ImGui::PushID(nodeId);
                NavigationNode node = named.graph.getNode(nodeId);
                std::string nodeLabel = std::to_string(nodeId) + " at " +
                                        std::to_string(static_cast<int>(node.position.x)) + "," +
                                        std::to_string(static_cast<int>(node.position.y));

                if (ImGui::Selectable(nodeLabel.c_str(), selectedNodeId == nodeId))
                {
                    selectedNodeId = nodeId;
                    selectedEdge.reset();
                }

                ImGui::Indent();
                for (const NavigationEdge &edge : named.graph.getOutgoingEdges(nodeId))
                {
                    ImGui::PushID(edge.toId);
                    std::pair<int, int> ends{edge.fromId, edge.toId};
                    std::string edgeLabel =
                        std::string(edge.type == EdgeType::Jump ? "jump" : "walk") +
                        " to " + std::to_string(edge.toId);

                    if (ImGui::Selectable(edgeLabel.c_str(), selectedEdge == ends))
                    {
                        selectedEdge = ends;
                        selectedNodeId.reset();
                    }
                    ImGui::PopID();
                }
                ImGui::Unindent();
                ImGui::PopID();
            }

            ImGui::Unindent();
        }

        ImGui::PopID();
    }
}

const NavigationGraph *LevelEditorUi::selectedGraph(const Level &level) const
{
    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    if (selectedGraphIndex >= graphs.size())
        return graphs.empty() ? nullptr : &graphs.front().graph;

    return &graphs[selectedGraphIndex].graph;
}

std::optional<int> LevelEditorUi::getSelectedNodeId() const
{
    return selectedNodeId;
}

std::optional<std::pair<int, int>> LevelEditorUi::getSelectedEdge() const
{
    return selectedEdge;
}

void LevelEditorUi::update(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    Level &level)
{
    if (!editing)
        return;

    ImVec2 mouseScreenPosition = ImGui::GetMousePos();
    glm::vec2 worldPosition = imGuiManager.screenToWorld(mouseScreenPosition, camera.getZoom(), camera.getTopLeftPosition());
    glm::ivec2 tilePosition = level.getTileMap().worldToTilePosition(worldPosition);
    if (!level.getTileMap().validTilePosition(tilePosition))
        return;

    if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && !imGuiManager.getIO().WantCaptureMouse)
    {
        if (editingPlayerStartTile)
        {
            level.setPlayerStartTile(tilePosition);
            editingPlayerStartTile = false;
        }
        else if (level.getTileMap().tilePositionToTileIndex(tilePosition) != selectedTileIndex)
        {
            level.getTileMap().setTileIndex(tilePosition, selectedTileIndex);
            level.rebuildGraphs();
        }
    }
}