#pragma once

#include <memory>
#include <vector>
#include "ui/editor_ui.hpp"
#include "ui/imgui_manager.hpp"
#include "ui/editor_commands.hpp"

struct GameData;
class Camera2D;
class Level;
class Levels;
class Npc;
class Player;
class ScoringSystem;
class TextureCache;
class Window;

struct GameUiSubject
{
    GameData &gameData;
    Level &level;
    const std::vector<std::unique_ptr<Npc>> &npcs;
    const Player &player;
    const TextureCache &textures;
    Levels &levels;
    const Camera2D &camera;
    const ScoringSystem &scoringSystem;
    bool paused = false;
    bool showEditors = false;
};

class GameUi
{
public:
    GameUi(Window &window, int width, int height);

    void draw(const GameUiSubject &subject);
    void update(float deltaTime, Level &level, const Camera2D &camera);
    void resize(int width, int height);
    void valuesReplaced();

    EditorCommands &commands();

private:
    ImGuiManager imGuiManager;
    EditorUi editorUi;
};
