#include <imgui.h>
#include "ui/player_ui.hpp"
#include "ui/debug_aabb_overlay.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "ui/editor_commands.hpp"
#include "game/game_data.hpp"

void PlayerUi::draw(GameData &gameData, EditorCommands &commands)
{
    if (ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("Collider", &drawPlayerCollider);
        ImGui::SameLine();
        ImGui::Checkbox("Collisions", &drawPlayerCollisions);
        ImGui::SameLine();
        ImGui::Checkbox("Probes", &drawContactProbes);
    }

    if (ImGui::Button("Respawn"))
        commands.onRespawn();

    ImGui::Separator();
    drawSaveControls(saveable, "player", gameData.playerData, savePlayerData);
    ImGui::Separator();
    inspector::drawFields(gameData.playerData);
}

void PlayerUi::update(float deltaTime)
{
    fadingAABBs.update(deltaTime);
}

void PlayerUi::drawOverlay(
    const ImGuiManager &imGuiManager,
    const Camera2D &camera,
    const Player &player)
{
    if (drawPlayerCollider)
        ::drawPlayerCollider(imGuiManager, camera, player);

    if (drawPlayerCollisions)
        ::drawPlayerCollisions(player, fadingAABBs);

    if (drawContactProbes)
        ::drawContactProbes(imGuiManager, camera, player, fadingAABBs);

    drawFadingAABBs(imGuiManager, camera, fadingAABBs);
}
void PlayerUi::save(GameData &gameData)
{
    savePlayerData(gameData.playerData);
    saveable.saved("player", asJson(gameData.playerData));
}

bool PlayerUi::hasUnsavedChanges(const GameData &gameData) const
{
    return saveable.unsaved("player", asJson(gameData.playerData));
}

void PlayerUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
