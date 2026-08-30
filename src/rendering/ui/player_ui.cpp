#include <format>
#include <string>
#include <imgui.h>
#include "rendering/ui/player_ui.hpp"
#include "rendering/ui/data_inspector.hpp"
#include "rendering/ui/editor_commands.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "game/game_data.hpp"

void PlayerUi::draw(
    GameData &gameData,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &actorState,
    EditorCommands &commands)
{
    if (ImGui::CollapsingHeader("Overlays", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::Checkbox("AABBs", &drawPlayerAABBs);
        ImGui::SameLine();
        ImGui::Checkbox("Probes", &drawContactProbes);
    }

    if (ImGui::Button("Respawn"))
        commands.onRespawn();

    if (ImGui::BeginTable("Inspector", 2, ImGuiTableFlags_BordersInnerV))
    {
        auto drawRow = [](const char *label, const std::string &value)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(label);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(value.c_str());
        };

        drawRow(
            "Velocity",
            std::format(
                "{:.2f}, {:.2f}", playerMotionState.velocity.x, playerMotionState.velocity.y));
        drawRow("Position", std::format("{:.2f}, {:.2f}", playerPosition.x, playerPosition.y));

        drawRow("On Ground", playerMotionState.contacts.onGround ? "true" : "false");
        drawRow("Facing Left", actorState.facingLeft ? "true" : "false");
        drawRow(
            "Touching Left Wall", playerMotionState.contacts.touchingLeftWall ? "true" : "false");
        drawRow(
            "Touching Right Wall", playerMotionState.contacts.touchingRightWall ? "true" : "false");
        drawRow("Hit Ceiling", playerMotionState.contacts.hitCeiling ? "true" : "false");

        drawRow("Wall Sliding", playerMotionState.wallSlide.active ? "true" : "false");
        drawRow("Wall Jumping", playerMotionState.wallJump.active ? "true" : "false");
        drawRow("Dashing", playerMotionState.dash.active ? "true" : "false");
        drawRow("Hanging", playerMotionState.wallHang.active ? "true" : "false");

        drawRow("Animation", toString(actorState.currentAnimationState));

        ImGui::EndTable();
    }

    ImGui::Separator();
    saveable.drawControls("player", gameData.playerData, savePlayerData);
    ImGui::Separator();
    inspector::drawFields(gameData.playerData);
}

bool PlayerUi::drawsPlayerAABBs() const
{
    return drawPlayerAABBs;
}

bool PlayerUi::drawsContactProbes() const
{
    return drawContactProbes;
}

void PlayerUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
