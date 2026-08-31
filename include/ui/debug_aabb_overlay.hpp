#pragma once

#include "ui/actors_in_level.hpp"

class ImGuiManager;
class Camera2D;
class Player;
class Level;
class FadingAABBs;

void drawPlayerCollider(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player);

void drawPlayerCollisions(const Player &player, FadingAABBs &fadingAABBs);

void drawTileColliders(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level);

void drawLevelBounds(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level);

void drawSpawnOf(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    ActorShown showing);

void drawContactProbes(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player,
    FadingAABBs &fadingAABBs);

void drawFadingAABBs(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const FadingAABBs &fadingAABBs);
