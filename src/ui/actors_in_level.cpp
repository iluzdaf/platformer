#include <cfloat>
#include <cstddef>
#include <map>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/actors_in_level.hpp"
#include "actor/actor_animation_state.hpp"
#include "actor/actor_motion_state.hpp"
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "navigation/navigation_path.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"

namespace
{
    std::string labelOf(const NpcSpawnData &spawn, size_t index)
    {
        return spawn.type + " " + std::to_string(index + 1);
    }

    std::string labelOf(ActorShown shown, const std::vector<NpcSpawnData> &spawns)
    {
        switch (shown.what)
        {
        case ActorShown::What::Player:
            return "player";

        case ActorShown::What::Npc:
            if (shown.npcIndex < spawns.size())
                return labelOf(spawns[shown.npcIndex], shown.npcIndex);
            break;

        case ActorShown::What::None:
            break;
        }

        return "none";
    }

    void drawRow(const char *label, const std::string &value)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(value.c_str());
    }

    void drawThePlayer(
        const ActorMotionState &motion,
        const glm::vec2 &position,
        const ActorState &state)
    {
        if (!ImGui::BeginTable("Player", 2, ImGuiTableFlags_BordersInnerV))
            return;

        drawRow("Velocity", std::format("{:.2f}, {:.2f}", motion.velocity.x, motion.velocity.y));
        drawRow("Position", std::format("{:.2f}, {:.2f}", position.x, position.y));
        drawRow("Facing Left", state.facingLeft ? "true" : "false");
        drawRow("Wall Sliding", motion.wallSlide.active ? "true" : "false");
        drawRow("Wall Jumping", motion.wallJump.active ? "true" : "false");
        drawRow("Dashing", motion.dash.active ? "true" : "false");
        drawRow("Hanging", motion.wallHang.active ? "true" : "false");
        drawRow("Animation", toString(state.currentAnimationState));

        ImGui::EndTable();
    }

    void drawNpcState(const Level &level, const NpcSpawnData &spawn, const Npc *npc)
    {
        std::optional<std::pair<glm::vec2, glm::vec2>> beat = level.patrolFor(spawn);
        if (npc && beat &&
            !canPatrolBetween(
                level.graphFor(npc->getNavigationProfile()), beat->first, beat->second))
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "it cannot get back from there");

        std::string_view state = npc ? npc->getStateName() : std::string_view{};
        if (!state.empty())
            ImGui::Text("%.*s", static_cast<int>(state.size()), state.data());

        if (npc)
            ImGui::Text("at %.0f,%.0f", npc->getPosition().x, npc->getPosition().y);
        else
            ImGui::TextDisabled("not spawned");
    }

    void drawNpcEditing(const NpcSpawnData &spawn)
    {
        ImGui::Text("spawns at %d,%d", spawn.tilePosition.x, spawn.tilePosition.y);

        if (spawn.patrol)
            ImGui::Text(
                "beat %d,%d to %d,%d",
                spawn.patrol->from.x,
                spawn.patrol->from.y,
                spawn.patrol->to.x,
                spawn.patrol->to.y);
        else
            ImGui::TextDisabled("no beat");
    }
}

ActorAsked drawActorsInLevel(
    const Level &level,
    const std::vector<std::unique_ptr<Npc>> &npcs,
    const ActorMotionState &playerMotionState,
    const glm::vec2 &playerPosition,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcTypes,
    ActorShown showing)
{
    ActorAsked asked{showing, false, std::nullopt};
    const std::vector<NpcSpawnData> &spawns = level.getNpcs();
    if (showing.what == ActorShown::What::Npc && showing.npcIndex >= spawns.size())
        asked.show = ActorShown{};

    const ImGuiStyle &style = ImGui::GetStyle();
    float buttons = ImGui::CalcTextSize("add").x + ImGui::CalcTextSize("remove").x +
                    style.FramePadding.x * 4.0f + style.ItemSpacing.x * 2.0f;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttons);
    if (ImGui::BeginCombo("##actor", labelOf(showing, spawns).c_str()))
    {
        if (ImGui::Selectable("none", showing.what == ActorShown::What::None))
            asked.show = ActorShown{ActorShown::What::None, 0};

        if (ImGui::Selectable("player", showing.what == ActorShown::What::Player))
            asked.show = ActorShown{ActorShown::What::Player, 0};

        for (size_t index = 0; index < spawns.size(); ++index)
            if (ImGui::Selectable(
                    labelOf(spawns[index], index).c_str(),
                    showing == ActorShown{ActorShown::What::Npc, index}))
                asked.show = ActorShown{ActorShown::What::Npc, index};

        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("add"))
        ImGui::OpenPopup("##addNpc");

    if (ImGui::BeginPopup("##addNpc"))
    {
        for (const auto &[type, npcData] : npcTypes)
            if (ImGui::Selectable(type.c_str()))
                asked.addNpcOfType = type;

        ImGui::EndPopup();
    }

    ImGui::SameLine();
    ImGui::BeginDisabled(
        showing.what != ActorShown::What::Npc || showing.npcIndex >= spawns.size());
    if (ImGui::Button("remove"))
        asked.removeShownNpc = true;
    ImGui::EndDisabled();

    switch (showing.what)
    {
    case ActorShown::What::Player:
        ImGui::Separator();
        ImGui::Text("spawns at %d,%d", level.getPlayerStartTile().x, level.getPlayerStartTile().y);
        drawThePlayer(playerMotionState, playerPosition, playerState);
        break;

    case ActorShown::What::Npc:
        if (showing.npcIndex >= spawns.size())
            break;

        ImGui::Separator();
        drawNpcEditing(spawns[showing.npcIndex]);
        drawNpcState(
            level,
            spawns[showing.npcIndex],
            showing.npcIndex < npcs.size() ? npcs[showing.npcIndex].get() : nullptr);
        break;

    case ActorShown::What::None:
        break;
    }

    return asked;
}
