#pragma once

#include <memory>
#include <optional>
#include <vector>
#include "ui/editor_commands.hpp"
#include "ui/brush.hpp"
#include "ui/navigation_ui.hpp"
#include "ui/saveable.hpp"

class ImGuiManager;
class TileMap;
class Level;
class Texture2D;
class Camera2D;
struct GameData;
class Npc;

class LevelUi
{
public:
    void draw(
        Level &level,
        const std::vector<std::unique_ptr<Npc>> &npcs,
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

    bool hasUnsavedChanges(const Level &level) const;
    bool drawsTileColliders() const;
    bool drawsLevelBounds() const;
    bool drawsPlayerStart() const;
    void valuesReplaced();

private:
    Saveable saveable;
    NavigationUi navigationUi;
    bool drawGrid = false, drawTileInfo = false, drawTileColliders = false, drawLevelBounds = false,
         drawPlayerStart = false;

    void drawLevel(Level &level, EditorCommands &commands);
    void drawTileMap(
        Level &level,
        const Texture2D &tileSet,
        const GameData &gameData,
        std::optional<Brush> &brush);
};