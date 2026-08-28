#pragma once

#include <optional>
#include <memory>
#include <string>
#include <vector>
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_section.hpp"

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;
class Npc;

class LevelEditorUi
{
public:
    void draw(
        EditorSection section,
        Level &level,
        const Texture2D &tileSet,
        const std::string &firstLevel,
        EditorCommands &commands);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;
    void update(const ImGuiManager &imGuiManager, const Camera2D &camera, Level &level);

    bool drawsTileMapAABBs() const;

private:
    bool editing = false, editingPlayerStartTile = false;
    int selectedTileIndex = 0;
    bool drawGrid = false, drawTileInfo = false, drawTileMapAABBs = false;

    void drawLevel(Level &level, const std::string &firstLevel, EditorCommands &commands);
    void drawTileMap(Level &level, const Texture2D &tileSet);
    std::optional<std::string> drawLevelChooser(
        const Level &level,
        const std::string &firstLevel,
        EditorCommands &commands);
};