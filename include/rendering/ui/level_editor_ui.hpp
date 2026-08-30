#pragma once

#include <optional>
#include <memory>
#include <string>
#include <vector>
#include "rendering/ui/editor_commands.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/ui/brush.hpp"
#include "rendering/ui/saveable.hpp"

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;
struct GameData;
class Npc;

class LevelEditorUi
{
public:
    void draw(
        EditorSection section,
        Level &level,
        const Texture2D &tileSet,
        const GameData &gameData,
        std::optional<Brush> &brush,
        EditorCommands &commands);
    void drawOverlay(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
        const;
    void update(
        const ImGuiManager &imGuiManager,
        const Camera2D &camera,
        Level &level,
        std::optional<Brush> &brush);

    bool drawsTileMapAABBs() const;
    void valuesReplaced();

private:
    Saveable saveable;
    bool drawGrid = false, drawTileInfo = false, drawTileMapAABBs = false;

    void drawLevel(Level &level, EditorCommands &commands);
    void drawTileMap(
        Level &level,
        const Texture2D &tileSet,
        const GameData &gameData,
        std::optional<Brush> &brush);
};