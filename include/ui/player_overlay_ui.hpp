#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include "ui/fading_aabbs.hpp"

class Camera2D;
class ImGuiManager;
class Player;

class PlayerOverlayUi
{
public:
    void drawToggles();
    void update(float deltaTime);
    void draw(const ImGuiManager &imGuiManager, const Camera2D &camera, const Player &player);

private:
    FadingAABBs fadingAABBs;
    bool playerShown = false;
};
