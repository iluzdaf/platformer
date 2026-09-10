#include <cfloat>
#include <cstddef>
#include <map>
#include <format>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/actors_in_level.hpp"
#include "ui/armed.hpp"
#include "actor/observed.hpp"
#include "actor/actor_state.hpp"
#include "game/level.hpp"
#include "navigation/navigation_place.hpp"
#include "npc/npc.hpp"
#include "physics/physics_body.hpp"
#include "tile_map/tile_map.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "game/beat_between.hpp"
#include "ui/state_machine_graph.hpp"
#include "ui/animator_field.hpp"
#include "actor/actor_data.hpp"
#include "actor/actor_animation_data.hpp"
#include "ui/state_machine_shown.hpp"
#include "conditions/asked.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    std::string labelOf(const NpcSpawnData &spawn, size_t index)
    {
        return spawn.type + " " + std::to_string(index + 1);
    }

    std::string labelOf(ActorShown shown, const std::vector<std::unique_ptr<Npc>> &npcs)
    {
        switch (shown.what)
        {
        case ActorShown::What::Player:
            return "player";

        case ActorShown::What::Npc:
            if (shown.npcIndex < npcs.size())
                return labelOf(npcs[shown.npcIndex]->getSpawn(), shown.npcIndex);
            break;

        case ActorShown::What::None:
            break;
        }

        return "none";
    }

    void nameThenValue()
    {
        ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);
    }

    void beginRow(const char *label)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(label);
        ImGui::TableSetColumnIndex(1);
    }

    void drawRow(const char *label, const std::string &value)
    {
        beginRow(label);
        ImGui::TextUnformatted(value.c_str());
    }

    void drawThePlayer(const Observed &observed, const glm::vec2 &feet, const ActorState &state)
    {
        drawRow(
            "Velocity", std::format("{:.2f}, {:.2f}", observed.velocity.x, observed.velocity.y));
        drawRow("Feet", std::format("{:.2f}, {:.2f}", feet.x, feet.y));
        drawRow("Facing Left", state.facingLeft ? "true" : "false");
    }

    void drawAnimatorOfThePlayer(const ActorAnimationData &animations, const ActorState &state)
    {
        if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
            drawAnimatorGraph(animations, {state.currentAnimation}, MachineShown{});
    }

    void drawCannotGetBack(const Level &level, const Npc *npc)
    {
        const std::optional<PatrolData> &beat = npc ? npc->getSpawn().patrol : std::nullopt;
        if (npc && beat && !canPatrolBetween(level.graphFor(npc->profile()), beat->from, beat->to))
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "it cannot get back from there");
    }

    void drawNpcState(const Level &level, const Npc *npc)
    {
        if (!npc)
        {
            beginRow("State");
            ImGui::TextDisabled("not spawned");
            return;
        }

        glm::ivec2 on = level.getTileMap().tileStoodOnAt(npc->feet());
        drawRow("Stands On", std::format("{}, {}", on.x, on.y));

        std::optional<int> setOffAt = npc->currentNodeId();
        std::optional<int> headingFor = npc->targetNodeId();
        if (!setOffAt)
        {
            beginRow("Route");
            ImGui::TextDisabled("off the graph");
        }
        else if (!headingFor)
            drawRow("Route", std::format("waiting at {}", *setOffAt));
        else
            drawRow("Route", std::format("{} heading for {}", *setOffAt, *headingFor));
    }

    void drawMachineOf(const std::map<std::string, NpcData> &npcTypes, const Npc &npc)
    {
        auto type = npcTypes.find(npc.type());
        if (type == npcTypes.end())
            return;

        const std::optional<StateMachineBehaviorData> &machine =
            type->second.stateMachineBehaviorData;
        if (!machine.has_value())
            return;

        if (!ImGui::CollapsingHeader("Machine", ImGuiTreeNodeFlags_DefaultOpen))
            return;

        drawStateMachineGraph(machine.value(), {std::string(npc.stateName())}, MachineShown{});
        for (const auto &[name, value] : npc.facts())
            ImGui::Text("%s: %s", name.c_str(), textOf(value).c_str());
    }

    void drawAnimatorOf(const std::map<std::string, NpcData> &npcTypes, const Npc &npc)
    {
        auto type = npcTypes.find(npc.type());
        if (type == npcTypes.end())
            return;

        if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
            drawAnimatorGraph(
                type->second.actorData.animationData,
                {npc.state().currentAnimation},
                MachineShown{});
    }

    void drawArmButton(const char *label, PickTile pick, std::optional<Armed> &armed)
    {
        bool isArmed = armed && *armed == Armed{pick};

        if (isArmed)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ArmedColour);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ArmedColour);
        }

        bool clicked = ImGui::Button(label);

        if (isArmed)
            ImGui::PopStyleColor(3);

        if (clicked)
            armed = isArmed ? std::nullopt : std::optional<Armed>(pick);
    }

    std::string asTile(glm::ivec2 tile)
    {
        return std::to_string(tile.x) + "," + std::to_string(tile.y);
    }

    void drawTileArmButton(const std::string &shown, PickTile pick, std::optional<Armed> &armed)
    {
        drawArmButton((shown + "##" + pickId(pick)).c_str(), pick, armed);
    }

    void drawPlayerEditing(glm::ivec2 startTile, std::optional<Armed> &armed)
    {
        beginRow("Spawns At");
        drawTileArmButton(asTile(startTile), PickTile{PickTile::For::PlayerStart, 0}, armed);
    }

    bool drawNpcEditing(
        const TileMap &tileMap,
        const NpcSpawnData &spawn,
        size_t index,
        std::optional<Armed> &armed)
    {
        beginRow("Spawns At");
        drawTileArmButton(
            asTile(tileMap.tileUnderFeet(spawn.feet)),
            PickTile{PickTile::For::NpcSpawn, index},
            armed);

        std::pair<glm::ivec2, glm::ivec2> beat;
        if (spawn.patrol)
            beat = tilesOfBeat(tileMap, *spawn.patrol);

        beginRow("Beat");
        drawTileArmButton(
            spawn.patrol ? asTile(beat.first) : "from",
            PickTile{PickTile::For::PatrolFrom, index},
            armed);
        ImGui::SameLine();
        ImGui::TextUnformatted("to");
        ImGui::SameLine();
        drawTileArmButton(
            spawn.patrol ? asTile(beat.second) : "to",
            PickTile{PickTile::For::PatrolTo, index},
            armed);
        ImGui::SameLine();

        ImGui::BeginDisabled(!spawn.patrol);
        bool clearing = ImGui::Button("clear");
        ImGui::EndDisabled();

        return clearing;
    }
}

std::optional<std::string> npcsThatCannotGetBack(const Level &level)
{
    std::string names;
    const std::vector<std::unique_ptr<Npc>> &placed = level.getNpcs();
    for (std::size_t index = 0; index < placed.size(); ++index)
    {
        const std::optional<PatrolData> &beat = placed[index]->getSpawn().patrol;
        if (!beat ||
            canPatrolBetween(level.graphFor(placed[index]->profile()), beat->from, beat->to))
            continue;

        names += (names.empty() ? "" : ", ") + labelOf(placed[index]->getSpawn(), index);
    }

    if (names.empty())
        return std::nullopt;

    return names + " cannot get back from there";
}

ActorAsked drawActorsInLevel(
    const Level &level,
    const ActorAnimationData &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcTypes,
    ActorShown showing,
    std::optional<Armed> &armed)
{
    const std::vector<std::unique_ptr<Npc>> &npcs = level.getNpcs();
    if (showing.what == ActorShown::What::Npc && showing.npcIndex >= npcs.size())
        showing = ActorShown{};

    ActorAsked asked{showing, false, false, std::nullopt};

    const ImGuiStyle &style = ImGui::GetStyle();
    float buttons = ImGui::CalcTextSize("add").x + ImGui::CalcTextSize("remove").x +
                    style.FramePadding.x * 4.0f + style.ItemSpacing.x * 2.0f;

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - buttons);
    if (ImGui::BeginCombo("##actor", labelOf(showing, npcs).c_str()))
    {
        if (ImGui::Selectable("none", showing.what == ActorShown::What::None))
            asked.show = ActorShown{ActorShown::What::None, 0};

        if (ImGui::Selectable("player", showing.what == ActorShown::What::Player))
            asked.show = ActorShown{ActorShown::What::Player, 0};

        for (size_t index = 0; index < npcs.size(); ++index)
            if (ImGui::Selectable(
                    labelOf(npcs[index]->getSpawn(), index).c_str(),
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
    ImGui::BeginDisabled(showing.what != ActorShown::What::Npc);
    if (ImGui::Button("remove"))
        asked.removeShownNpc = true;
    ImGui::EndDisabled();

    switch (showing.what)
    {
    case ActorShown::What::Player:
        ImGui::Separator();
        if (ImGui::BeginTable("Player", 2, ImGuiTableFlags_BordersInnerV))
        {
            nameThenValue();
            drawPlayerEditing(level.getTileMap().tileUnderFeet(level.getPlayerStart()), armed);
            drawThePlayer(playerObserved, playerFeet, playerState);
            ImGui::EndTable();
        }

        drawAnimatorOfThePlayer(playerAnimations, playerState);
        break;

    case ActorShown::What::Npc: {
        const Npc *npc = npcs[showing.npcIndex].get();
        const NpcSpawnData &spawn = npc->getSpawn();

        ImGui::Separator();
        drawCannotGetBack(level, npc);

        if (ImGui::BeginTable("Npc", 2, ImGuiTableFlags_BordersInnerV))
        {
            nameThenValue();
            asked.clearShownBeat =
                drawNpcEditing(level.getTileMap(), spawn, showing.npcIndex, armed);
            drawNpcState(level, npc);
            ImGui::EndTable();
        }

        drawMachineOf(npcTypes, *npc);
        drawAnimatorOf(npcTypes, *npc);
        break;
    }

    case ActorShown::What::None:
        break;
    }

    return asked;
}
