#pragma once

#include <signals.hpp>
#include <optional>
#include <string>

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;
class NavigationGraph;

class LevelEditorUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        Level &level,
        const Texture2D &tileSet,
        const Camera2D &camera,
        const std::string &firstLevel,
        bool showEditors);
    void update(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        Level &level);

    fteng::signal<void(const std::string &)> onLoadLevel;
    fteng::signal<void()> onRespawn,
        onSetFirstLevel;

    bool drawsTileMapAABBs() const;
    const NavigationGraph *selectedGraph(const Level &level) const;
    std::optional<int> getSelectedNodeId() const;
    std::optional<std::pair<int, int>> getSelectedEdge() const;

private:
    bool editing = false,
         editingPlayerStartTile = false;
    int selectedTileIndex = 0;
    bool showNavigation = true,
         drawGrid = false,
         drawTileInfo = false,
         drawTileMapAABBs = false;
    size_t selectedGraphIndex = 0;
    std::optional<int> selectedNodeId;
    std::optional<std::pair<int, int>> selectedEdge;

    void drawGraphs(const Level &level);
    void drawEdgesOf(const NavigationGraph &graph, int nodeId);
    std::optional<std::string> drawLevelChooser(const Level &level, const std::string &firstLevel);
};