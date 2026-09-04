#pragma once

#include "game/levels_data.hpp"

#include "game/level_data.hpp"
#include <string>

#include "ui/editor_ui.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/editor_commands.hpp"

struct GameData;
class Camera2D;
class Level;
class Npc;
class Player;
class Score;
class TextureCache;
class Window;

struct GameUiSubject
{
    GameData &gameData;
    const Level &level;
    const LevelData &levelData;
    const std::string &levelPath;
    const Player &player;
    const TextureCache &textures;
    LevelsData &levels;
    const Camera2D &camera;
    const Score &score;
    bool paused = false;
    bool showEditors = false;
};

class GameUi
{
public:
    GameUi(Window &window, int width, int height);

    void draw(const GameUiSubject &subject);
    void update(
        float deltaTime,
        const Level &level,
        const LevelData &levelData,
        const std::string &levelPath,
        const Camera2D &camera);
    void resize(int width, int height);
    EditorUi &editor();
    EditorCommands &commands();

private:
    ImGuiManager imGuiManager;
    EditorUi editorUi;
};
