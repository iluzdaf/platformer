#include <imgui.h>
#include <format>
#include <string>
#include "rendering/ui/game_editor_ui.hpp"
#include "actor/actor_animation_state.hpp"
#include "rendering/ui/imgui_manager.hpp"
#include "rendering/ui/editor_ui.hpp"
#include "rendering/ui/editor_section.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "cameras/camera2d.hpp"

void GameEditorUi::draw(
    const ImGuiManager &imGuiManager,
    EditorSection section,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &actorState,
    const Camera2D &camera,
    bool showEditors)
{
    if (!showEditors)
        return;

    if (section == EditorSection::Playback)
    {
        EditorUi::beginInspector(imGuiManager, "Playback");
        if (ImGui::Button("Step"))
            onStep();
        ImGui::SameLine();
        if (ImGui::Button("Play"))
            onPlay();
        EditorUi::endInspector();
        return;
    }

    if (section == EditorSection::Camera)
    {
        EditorUi::beginInspector(imGuiManager, "Camera");
        if (ImGui::Button("Zoom"))
            onToggleZoom();
        ImGui::SameLine();
        ImGui::Text("shaking %s", camera.shaking() ? "yes" : "no");
        EditorUi::endInspector();
        return;
    }

    if (section != EditorSection::Player)
        return;

    EditorUi::beginInspector(imGuiManager, "Player");
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

    EditorUi::endInspector();
}
bool GameEditorUi::drawsPlayerAABBs() const
{
    return drawPlayerAABBs;
}
