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
#include "pickups/pickup.hpp"
#include "pickups/pickup_spawn_data.hpp"
#include "game/beat_between.hpp"
#include "ui/state_machine_graph.hpp"
#include "ui/animator_field.hpp"
#include "actor/actor_data.hpp"
#include "animations/animator_data.hpp"
#include "ui/state_machine_shown.hpp"
#include "conditions/asked.hpp"
#include "actor/fading_facts.hpp"
#include "tile_map/tile_map.hpp"

namespace
{
    constexpr ImVec4 JustSaidColour{0.5f, 1.0f, 0.6f, 1.0f};

    std::string labelOf(const NpcSpawnData &spawn, size_t index)
    {
        return spawn.type + " " + std::to_string(index + 1);
    }

    std::string labelOf(const PickupSpawnData &spawn, size_t index)
    {
        return spawn.type + " " + std::to_string(index + 1);
    }

    std::string labelOf(
        ActorShown shown,
        const std::vector<std::unique_ptr<Npc>> &npcs,
        const std::vector<Pickup> &pickups)
    {
        switch (shown.what)
        {
        case ActorShown::What::Player:
            return "player";

        case ActorShown::What::Npc:
            if (shown.index < npcs.size())
                return labelOf(npcs[shown.index]->getSpawn(), shown.index);
            break;

        case ActorShown::What::Pickup:
            if (shown.index < pickups.size())
                return labelOf(pickups[shown.index].getSpawn(), shown.index);
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

    void drawAnimatorOfThePlayer(
        const std::optional<AnimatorData> &animations,
        const ActorState &state)
    {
        if (!animations)
            return;

        if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
            drawAnimatorGraph(*animations, {state.currentAnimation}, MachineShown{});
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

    void drawFact(const std::string &name, const Asked &value, const FadingFacts &lately)
    {
        auto said = lately.all().find(name);
        if (said == lately.all().end())
        {
            ImGui::Text("%s: %s", name.c_str(), textOf(value).c_str());
            return;
        }

        ImGui::TextColored(
            JustSaidColour, "%s: %s", name.c_str(), textOf(said->second.value).c_str());
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
            drawFact(name, value, npc.saidLately());
    }

    void drawAnimatorOf(const std::map<std::string, NpcData> &npcTypes, const Npc &npc)
    {
        auto type = npcTypes.find(npc.type());
        if (type == npcTypes.end() || !type->second.actorData.animationData)
            return;

        if (ImGui::CollapsingHeader("Animator", ImGuiTreeNodeFlags_DefaultOpen))
            drawAnimatorGraph(
                *type->second.actorData.animationData,
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

    void drawPickupEditing(
        const TileMap &tileMap,
        const Pickup &pickup,
        size_t index,
        std::optional<Armed> &armed)
    {
        beginRow("Spawns At");
        drawTileArmButton(
            asTile(tileMap.tileUnderFeet(pickup.getSpawn().feet)),
            PickTile{PickTile::For::PickupSpawn, index},
            armed);

        drawRow("Gives", std::to_string(pickup.getScoreDelta()));

        beginRow("State");
        if (pickup.stillThere())
            ImGui::TextUnformatted("waiting");
        else
            ImGui::TextDisabled("picked up");
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
    const std::optional<AnimatorData> &playerAnimations,
    const Observed &playerObserved,
    const glm::vec2 &playerFeet,
    const ActorState &playerState,
    const std::map<std::string, NpcData> &npcTypes,
    ActorShown showing,
    std::optional<Armed> &armed)
{
    const std::vector<std::unique_ptr<Npc>> &npcs = level.getNpcs();
    const std::vector<Pickup> &pickups = level.getPickups();
    if (showing.what == ActorShown::What::Npc && showing.index >= npcs.size())
        showing = ActorShown{};

    if (showing.what == ActorShown::What::Pickup && showing.index >= pickups.size())
        showing = ActorShown{};

    ActorAsked asked{showing, false, false};

    ImGui::PushID("inTheLevel");
    ImGui::TextUnformatted(labelOf(showing, npcs, pickups).c_str());
    ImGui::SameLine();
    ImGui::BeginDisabled(
        showing.what != ActorShown::What::Npc && showing.what != ActorShown::What::Pickup);
    if (ImGui::Button("remove", ImVec2(-FLT_MIN, 0.0f)))
        asked.removeShown = true;

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
        const Npc *npc = npcs[showing.index].get();
        const NpcSpawnData &spawn = npc->getSpawn();

        ImGui::Separator();
        drawCannotGetBack(level, npc);

        if (ImGui::BeginTable("Npc", 2, ImGuiTableFlags_BordersInnerV))
        {
            nameThenValue();
            asked.clearShownBeat = drawNpcEditing(level.getTileMap(), spawn, showing.index, armed);
            drawNpcState(level, npc);
            ImGui::EndTable();
        }

        drawMachineOf(npcTypes, *npc);
        drawAnimatorOf(npcTypes, *npc);
        break;
    }

    case ActorShown::What::Pickup: {
        ImGui::Separator();
        if (ImGui::BeginTable("Pickup", 2, ImGuiTableFlags_BordersInnerV))
        {
            nameThenValue();
            drawPickupEditing(level.getTileMap(), pickups[showing.index], showing.index, armed);
            ImGui::EndTable();
        }

        break;
    }

    case ActorShown::What::None:
        break;
    }

    ImGui::PopID();

    return asked;
}
