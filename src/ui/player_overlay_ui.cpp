#include <imgui.h>
#include "ui/player_overlay_ui.hpp"
#include "ui/debug_aabb_overlay.hpp"

void PlayerOverlayUi::drawToggles()
{
    ImGui::Checkbox("Player", &playerShown);
}

void PlayerOverlayUi::update(float deltaTime)
{
    fadingAABBs.update(deltaTime);
}

void PlayerOverlayUi::draw(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player)
{
    if (playerShown)
    {
        drawPlayerCollider(imGuiManager, camera, player);
        drawPlayerCollisions(player, fadingAABBs);
        drawContactProbes(imGuiManager, camera, player, fadingAABBs);
        drawPlayerHurting(imGuiManager, camera, player, fadingAABBs);
    }

    drawFadingAABBs(imGuiManager, camera, fadingAABBs);
}
