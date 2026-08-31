#include <optional>
#include <imgui.h>
#include "ui/debug_aabb_overlay.hpp"
#include "ui/fading_aabbs.hpp"
#include "ui/imgui_manager.hpp"
#include "game/level.hpp"
#include "actor/actor_motion_state.hpp"
#include "player/player.hpp"
#include "tile_map/tile_map.hpp"
#include <cstddef>
#include <string>
#include <vector>
#include <utility>
#include "cameras/camera2d.hpp"
#include "npc/npc_spawn_data.hpp"
#include "ui/actors_in_level.hpp"
#include "actor/actor_contact_state.hpp"
#include "physics/physics_body.hpp"
#include "physics/aabb.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_node.hpp"
#include "navigation/navigation_path.hpp"
#include "ui/navigation_overlay.hpp"

namespace
{
    constexpr float MinimumProbeDepth = 1.0f;
    constexpr ImU32 ProbeIdleColor = IM_COL32(0, 200, 230, 110);
    constexpr ImU32 ProbeFoundColor = IM_COL32(0, 220, 255, 255);
    constexpr ImU32 ProbeGripColor = IM_COL32(0, 220, 255, 70);
    constexpr ImU32 PlayerColliderColor = IM_COL32(0, 255, 0, 255);
    constexpr ImU32 PlayerCollisionColor = IM_COL32(255, 127, 0, 255);
    constexpr ImU32 TileColliderColor = IM_COL32(230, 230, 230, 255);
    constexpr ImU32 DeadlyTileColliderColor = IM_COL32(255, 0, 0, 255);
    constexpr ImU32 LevelBoundsColor = IM_COL32(255, 255, 0, 255);
    constexpr ImU32 SpawnColor = IM_COL32(255, 0, 255, 255);
    constexpr float BeatEndRadius = 3.0f;

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

    void drawProbe(
        ImDrawList *drawList,
        const ImGuiManager &imGuiManager,
        AABB probe,
        const Camera2D &camera,
        bool found,
        bool grippable)
    {
        if (grippable)
        {
            ImVec2 topLeft = imGuiManager.worldToScreen(
                probe.position, camera.getZoom(), camera.getTopLeftPosition());
            ImVec2 bottomRight = imGuiManager.worldToScreen(
                probe.position + probe.size, camera.getZoom(), camera.getTopLeftPosition());
            drawList->AddRectFilled(topLeft, bottomRight, ProbeGripColor);
        }
        drawAABB(drawList, imGuiManager, probe, camera, found ? ProbeFoundColor : ProbeIdleColor);
    }
}

void drawPlayerCollider(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player)
{
    drawAABB(
        ImGui::GetBackgroundDrawList(),
        imGuiManager,
        player.getPhysicsBody().getAABB(),
        camera,
        PlayerColliderColor);
}

void drawPlayerCollisions(const Player &player, FadingAABBs &fadingAABBs)
{
    ActorMotionState state = player.getMotion().getState();
    fadingAABBs.add(state.contacts.collisionAABBX, PlayerCollisionColor, 0.1f);
    fadingAABBs.add(state.contacts.collisionAABBY, PlayerCollisionColor, 0.1f);
}

void drawTileColliders(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
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
            tile.isDeadly() ? DeadlyTileColliderColor : TileColliderColor);
    }
}

void drawLevelBounds(const ImGuiManager &imGuiManager, const Camera2D &camera, const Level &level)
{
    const TileMap &tileMap = level.getTileMap();
    drawAABB(
        ImGui::GetBackgroundDrawList(),
        imGuiManager,
        AABB(glm::vec2(0), glm::vec2(tileMap.getWorldWidth(), tileMap.getWorldHeight())),
        camera,
        LevelBoundsColor);
}

void drawSpawnOf(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Level &level,
    ActorShown showing)
{
    const TileMap &tileMap = level.getTileMap();
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    float tileSize = static_cast<float>(tileMap.getTileSize());

    auto drawSpawn = [&](glm::ivec2 tilePosition, const std::string &label)
    {
        glm::vec2 worldPosition = tileMap.tileToWorldPosition(tilePosition);
        drawAABB(
            drawList, imGuiManager, AABB(worldPosition, glm::vec2(tileSize)), camera, SpawnColor);

        ImVec2 topLeft = imGuiManager.worldToScreen(
            worldPosition, camera.getZoom(), camera.getTopLeftPosition());
        drawList->AddText(ImVec2(topLeft.x + 2.0f, topLeft.y + 2.0f), SpawnColor, label.c_str());
    };

    if (showing.what == ActorShown::What::Player)
    {
        drawSpawn(level.getPlayerStartTile(), "player");
        return;
    }

    if (showing.what != ActorShown::What::Npc || showing.npcIndex >= level.getNpcs().size())
        return;

    const NpcSpawnData &spawn = level.getNpcs()[showing.npcIndex];
    drawSpawn(spawn.tilePosition, spawn.type);

    std::optional<std::pair<glm::vec2, glm::vec2>> beat = level.patrolFor(spawn);
    if (!beat)
        return;

    const NavigationGraph &graph = level.graphForNpc(spawn);
    std::optional<int> fromId = nearestNodeTo(graph, beat->first);
    std::optional<int> toId = nearestNodeTo(graph, beat->second);
    if (!fromId || !toId)
        return;

    auto onScreen = [&](int id)
    {
        return imGuiManager.worldToScreen(
            graph.getNode(id).position, camera.getZoom(), camera.getTopLeftPosition());
    };

    std::vector<int> route = findPath(graph, *fromId, *toId);
    for (std::size_t step = 1; step < route.size(); ++step)
        drawList->AddLine(onScreen(route[step - 1]), onScreen(route[step]), SpawnColor);

    drawList->AddCircleFilled(onScreen(*fromId), BeatEndRadius, OriginColour);
    drawList->AddCircleFilled(onScreen(*toId), BeatEndRadius, DestinationColour);
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

    AABB overhead = thickEnoughToSee(physicsBody.overheadProbe(), false);
    drawProbe(
        drawList,
        imGuiManager,
        thickEnoughToSee(physicsBody.underfootProbe(), true),
        camera,
        contacts.onGround,
        false);
    drawProbe(drawList, imGuiManager, overhead, camera, contacts.bumpedCeiling, false);
    if (contacts.bumpedCeiling)
        fadingAABBs.add(overhead, ProbeFoundColor, 0.2f);
    drawProbe(
        drawList,
        imGuiManager,
        physicsBody.wallProbe(-1.0f),
        camera,
        contacts.touchingLeftWall,
        contacts.grippableLeftWall);
    drawProbe(
        drawList,
        imGuiManager,
        physicsBody.wallProbe(1.0f),
        camera,
        contacts.touchingRightWall,
        contacts.grippableRightWall);
    drawProbe(
        drawList,
        imGuiManager,
        physicsBody.wallProbeAtHead(-1.0f),
        camera,
        contacts.touchingLeftWall && !contacts.ledgeOnLeft,
        false);
    drawProbe(
        drawList,
        imGuiManager,
        physicsBody.wallProbeAtHead(1.0f),
        camera,
        contacts.touchingRightWall && !contacts.ledgeOnRight,
        false);
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
