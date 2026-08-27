#include <algorithm>
#include "rendering/ui/level_editor_ui.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/debug_data.hpp"
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
    const std::string &firstLevel,
    DebugData &debug)
{
    if (!debug.showLevelEditor)
        return;

    ImVec2 displaySize = imGuiManager.getUiDimensions();
    ImGui::SetNextWindowPos(ImVec2(displaySize.x - 200, 0));
    ImGui::SetNextWindowSize(ImVec2(200, displaySize.y));
    ImGui::Begin("Level Editor");

    ImGui::Checkbox("Tile Info", &debug.shouldDrawTileInfo);
    ImGui::SameLine();
    ImGui::Checkbox("Grid", &debug.shouldDrawGrid);
    ImGui::Checkbox("TileMap", &debug.shouldDrawTileMapAABBs);
    ImGui::SameLine();
    if (ImGui::Button("Respawn"))
        onRespawn();
    ImGui::Separator();

    std::optional<std::string> chosenLevel = drawLevelChooser(level, firstLevel);
    if (chosenLevel)
    {
        editing = false;
        ImGui::End();
        onLoadLevel(*chosenLevel);
        return;
    }

    ImGui::Text(
        "w%dxh%dxs%d",
        level.getTileMap().getWidth(),
        level.getTileMap().getHeight(),
        level.getTileMap().getTileSize());

    ImGui::TextUnformatted("next");
    ImGui::SameLine();
    if (!editing)
        ImGui::TextUnformatted(levelName(level.getNextLevel()).c_str());
    else
    {
        ImGui::SetNextItemWidth(110.0f);
        if (ImGui::BeginCombo("##next", levelName(level.getNextLevel()).c_str()))
        {
            std::string directory =
                level.getPath().substr(0, level.getPath().find_last_of("/\\"));
            for (const std::string &path : levelPathsIn(directory))
                if (ImGui::Selectable(levelName(path).c_str(), path == level.getNextLevel()))
                    level.setNextLevel(path);

            ImGui::EndCombo();
        }
    }

    const std::vector<NpcSpawnData> &npcs = level.getNpcs();
    if (ImGui::CollapsingHeader("npcs", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Indent();
        if (npcs.empty())
            ImGui::TextDisabled("none");
        else
            for (const NpcSpawnData &npc : npcs)
                ImGui::TextUnformatted(npc.type.c_str());
        ImGui::Unindent();
    }

    drawGraphs(level);

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

std::optional<std::string> LevelEditorUi::drawLevelChooser(
    const Level &level,
    const std::string &firstLevel)
{
    std::string directory = level.getPath().substr(0, level.getPath().find_last_of("/\\"));
    std::optional<std::string> chosen;

    ImGui::SetNextItemWidth(110.0f);
    if (ImGui::BeginCombo("##level", levelName(level.getPath()).c_str()))
    {
        for (const std::string &path : levelPathsIn(directory))
        {
            bool current = path == level.getPath();
            if (ImGui::Selectable(levelName(path).c_str(), current) && !current)
                chosen = path;
        }
        ImGui::EndCombo();
    }

    if (editing)
    {
        ImGui::SameLine();
        bool isFirst = level.getPath() == firstLevel;
        if (ImGui::Checkbox("first", &isFirst) && isFirst)
            onSetFirstLevel();
    }

    return chosen;
}

void LevelEditorUi::drawGraphs(const Level &level)
{
    if (!ImGui::CollapsingHeader("navigation", ImGuiTreeNodeFlags_DefaultOpen))
        return;

    const std::vector<NamedNavigationGraph> &graphs = level.getGraphs();
    ImGui::Indent();

    if (graphs.empty())
    {
        ImGui::TextDisabled("none");
        ImGui::Unindent();
        return;
    }

    if (selectedGraphIndex >= graphs.size())
        selectedGraphIndex = 0;

    const NamedNavigationGraph &shown = graphs[selectedGraphIndex];
    std::string preview = shown.name + " (" +
                          std::to_string(shown.graph.getNodes().size()) + " nodes, " +
                          std::to_string(shown.graph.getEdges().size()) + " edges)";

    ImGui::SetNextItemWidth(170.0f);
    if (ImGui::BeginCombo("##graph", preview.c_str()))
    {
        for (size_t index = 0; index < graphs.size(); ++index)
            if (ImGui::Selectable(graphs[index].name.c_str(), index == selectedGraphIndex))
            {
                selectedGraphIndex = index;
                selectedNodeId.reset();
                selectedEdge.reset();
            }

        ImGui::EndCombo();
    }

    std::vector<int> nodeIds;
    for (const auto &[nodeId, node] : shown.graph.getNodes())
        nodeIds.push_back(nodeId);
    std::sort(nodeIds.begin(), nodeIds.end());

    for (int nodeId : nodeIds)
    {
        ImGui::PushID(nodeId);
        NavigationNode node = shown.graph.getNode(nodeId);
        std::string nodeLabel = std::to_string(nodeId) + " at " +
                                std::to_string(static_cast<int>(node.position.x)) + "," +
                                std::to_string(static_cast<int>(node.position.y));

        if (ImGui::Selectable(nodeLabel.c_str(), selectedNodeId == nodeId))
        {
            selectedNodeId = nodeId;
            selectedEdge.reset();
        }

        ImGui::Indent();
        drawEdgesOf(shown.graph, nodeId);
        ImGui::Unindent();
        ImGui::PopID();
    }

    ImGui::Unindent();
}

void LevelEditorUi::drawEdgesOf(const NavigationGraph &graph, int nodeId)
{
    auto drawEdge = [&](const NavigationEdge &edge, bool leaving)
    {
        ImGui::PushID(leaving ? edge.toId : -edge.fromId - 1);
        std::pair<int, int> ends{edge.fromId, edge.toId};
        std::string label = std::string(edge.type == EdgeType::Jump ? "jump" : "walk") +
                            (leaving ? " to " : " from ") +
                            std::to_string(leaving ? edge.toId : edge.fromId);

        if (ImGui::Selectable(label.c_str(), selectedEdge == ends))
        {
            selectedEdge = ends;
            selectedNodeId.reset();
        }
        ImGui::PopID();
    };

    for (const NavigationEdge &edge : graph.getOutgoingEdges(nodeId))
        drawEdge(edge, true);

    for (const NavigationEdge &edge : graph.getEdges())
        if (edge.toId == nodeId)
            drawEdge(edge, false);
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