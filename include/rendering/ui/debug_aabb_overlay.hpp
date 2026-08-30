#pragma once

class ImGuiManager;
class Camera2D;
class Player;
class Level;
class FadingAABBs;

void drawPlayerAABBs(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player,
    FadingAABBs &fadingAABBs);

void drawTileMapAABBs(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level);

void drawContactProbes(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player);

void drawFadingAABBs(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const FadingAABBs &fadingAABBs);
