#include <imgui.h>
#include <format>
#include <string>
#include "rendering/ui/game_editor_ui.hpp"
#include "actor/actor_animation_state.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"

void GameEditorUi::draw(
    const ImGuiManager &imGuiManager,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &actorState,
    const Camera2D &camera,
    bool showEditors)
{
    if (!showEditors)
        return;

    ImGui::SetNextWindowSize(ImVec2(200, imGuiManager.getUiDimensions().y));
    ImGui::Begin("Game Editor");
    if (ImGui::CollapsingHeader("Playback", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("play");
        if (ImGui::Button("Step"))
            onStep();
        ImGui::SameLine();
        if (ImGui::Button("Play"))
            onPlay();
        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::PushID("camera");
        if (ImGui::Button("Zoom"))
            onToggleZoom();
        ImGui::SameLine();
        ImGui::Text("shaking %s", camera.shaking() ? "yes" : "no");
        ImGui::PopID();
    }

    if (!ImGui::CollapsingHeader("Player", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::End();
        return;
    }

    ImGui::Checkbox("AABBs", &drawPlayerAABBs);

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
        drawRow("Climbing", playerMotionState.climb.active ? "true" : "false");

        drawRow("Animation", toString(actorState.currentAnimationState));

        ImGui::EndTable();
    }

    ImGui::End();
}
bool GameEditorUi::drawsPlayerAABBs() const
{
    return drawPlayerAABBs;
}
