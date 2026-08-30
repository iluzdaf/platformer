#include <optional>
#include <imgui.h>
#include "rendering/ui/debug_aabb_overlay.hpp"
#include "rendering/ui/fading_aabbs.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "game/level.hpp"
#include "actor/actor_motion_state.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include "cameras/camera2d.hpp"
#include "actor/actor_contact_state.hpp"
#include "physics/physics_body.hpp"
#include "physics/aabb.hpp"

namespace
{
    constexpr float MinimumProbeDepth = 1.0f;

    AABB thickEnoughToSee(AABB probe, bool surfaceIsTopEdge)
    {
        if (probe.size.y >= MinimumProbeDepth)
            return probe;

        glm::vec2 size(probe.size.x, MinimumProbeDepth);
        if (surfaceIsTopEdge)
            return AABB(probe.position, size);
        return AABB(
            glm::vec2(probe.position.x, probe.position.y + probe.size.y - MinimumProbeDepth), size);
    }

    void drawAABB(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        AABB aabb,
        const Camera2D &camera,
        ImU32 color)
    {
        if (aabb.isEmpty())
        {
            return;
        }
        ImVec2 topLeft = imGuiManager.worldToScreen(
            aabb.position, camera.getZoom(), camera.getTopLeftPosition());
        ImVec2 bottomRight = imGuiManager.worldToScreen(
            aabb.position + aabb.size, camera.getZoom(), camera.getTopLeftPosition());
        drawList->AddRect(topLeft, bottomRight, color);
    }
}

void drawPlayerAABBs(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player,
    FadingAABBs &fadingAABBs)
{
    drawAABB(
        ImGui::GetBackgroundDrawList(),
        imGuiManager,
        player.getPhysicsBody().getAABB(),
        camera,
        IM_COL32(0, 255, 0, 255));

    ActorMotionState state = player.getMotion().getState();
    fadingAABBs.add(state.contacts.collisionAABBX, IM_COL32(255, 255, 0, 255), 0.1f);
    fadingAABBs.add(state.contacts.collisionAABBY, IM_COL32(255, 127, 0, 255), 0.1f);
}

void drawTileMapAABBs(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    const TileMap &tileMap = level.getTileMap();

    auto tilePositions =
        tileMap.worldToTilePositions(camera.getTopLeftPosition(), camera.getWindowSize());
    for (auto tilePosition : tilePositions)
    {
        auto tile = tileMap.getTileAtTilePosition(tilePosition);
        if (tile.isSolid() || tile.isEmpty())
        {
            continue;
        }

        glm::vec2 tileWorldPosition = tileMap.tileToWorldPosition(tilePosition);
        std::optional<AABB> tileAABB = tile.getAABBAt(tileWorldPosition);
        if (!tileAABB)
            continue;

        drawAABB(
            drawList,
            imGuiManager,
            *tileAABB,
            camera,
            tile.isDeadly() ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255));
    }

    drawAABB(
        drawList,
        imGuiManager,
        AABB(glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight())),
        camera,
        IM_COL32(255, 255, 0, 255));

    glm::vec2 playerStartWorldPosition = tileMap.tileToWorldPosition(level.getPlayerStartTile());
    drawAABB(
        drawList,
        imGuiManager,
        AABB(playerStartWorldPosition, glm::vec2(static_cast<float>(tileMap.getTileSize()))),
        camera,
        IM_COL32(255, 0, 255, 255));
}

void drawContactProbes(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player,
    FadingAABBs &fadingAABBs)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    const PhysicsBody &physicsBody = player.getPhysicsBody();
    ActorContactState contacts = player.getMotion().getState().contacts;

    auto probeColor = [](bool touching, bool grippable)
    {
        if (grippable)
            return IM_COL32(0, 255, 128, 255);
        if (touching)
            return IM_COL32(0, 191, 255, 255);
        return IM_COL32(128, 128, 128, 128);
    };

    AABB overhead = thickEnoughToSee(physicsBody.overheadProbe(), false);
    drawAABB(
        drawList,
        imGuiManager,
        thickEnoughToSee(physicsBody.underfootProbe(), true),
        camera,
        probeColor(contacts.onGround, false));
    drawAABB(drawList, imGuiManager, overhead, camera, probeColor(contacts.hitCeiling, false));
    if (contacts.hitCeiling)
        fadingAABBs.add(overhead, probeColor(true, false), 0.2f);
    drawAABB(
        drawList,
        imGuiManager,
        physicsBody.wallProbe(-1.0f),
        camera,
        probeColor(contacts.touchingLeftWall, contacts.grippableLeftWall));
    drawAABB(
        drawList,
        imGuiManager,
        physicsBody.wallProbe(1.0f),
        camera,
        probeColor(contacts.touchingRightWall, contacts.grippableRightWall));
    drawAABB(
        drawList,
        imGuiManager,
        physicsBody.wallProbeAtHead(-1.0f),
        camera,
        probeColor(contacts.touchingLeftWall && !contacts.ledgeOnLeft, false));
    drawAABB(
        drawList,
        imGuiManager,
        physicsBody.wallProbeAtHead(1.0f),
        camera,
        probeColor(contacts.touchingRightWall && !contacts.ledgeOnRight, false));
}

void drawFadingAABBs(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const FadingAABBs &fadingAABBs)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    for (const auto &[hash, fading] : fadingAABBs.all())
    {
        drawAABB(drawList, imGuiManager, fading.box, camera, fading.color);
    }
}
