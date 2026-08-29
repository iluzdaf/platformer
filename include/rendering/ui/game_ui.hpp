#pragma once

#include <memory>
#include <string>
#include <vector>
#include "rendering/ui/debug_aabb_ui.hpp"
#include "rendering/ui/editor_ui.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/score_ui.hpp"

struct GameData;
class Camera2D;
class Level;
class Npc;
class Player;
class ScoringSystem;
class Texture2D;
class Window;

struct GameUiSubject
{
    GameData &gameData;
    Level &level;
    const std::vector<std::unique_ptr<Npc>> &npcs;
    const Player &player;
    const Texture2D &tileSet;
    const std::string &firstLevel;
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
    DebugAABBUi debugAABBUi;
    ScoreUi scoreUi;
};
