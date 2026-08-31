#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "ui/npcs_in_level.hpp"
#include "game/level.hpp"
#include "navigation/navigation_path.hpp"
#include "npc/npc.hpp"
#include "npc/npc_spawn_data.hpp"

void drawNpcsInLevel(const Level &level, const std::vector<std::unique_ptr<Npc>> &npcs)
{
    const std::vector<NpcSpawnData> &spawns = level.getNpcs();
    if (spawns.empty())
    {
        ImGui::TextDisabled("none");
        return;
    }

    for (size_t index = 0; index < spawns.size(); ++index)
    {
        const NpcSpawnData &spawn = spawns[index];
        const Npc *npc = index < npcs.size() ? npcs[index].get() : nullptr;

        ImGui::PushID(static_cast<int>(index));
        std::string_view state = npc ? npc->getStateName() : std::string_view{};
        std::string heading = spawn.type;
        if (!state.empty())
            heading += " - " + std::string(state);

        if (ImGui::TreeNodeEx(heading.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("spawns at %d,%d", spawn.tilePosition.x, spawn.tilePosition.y);

            if (spawn.patrol)
            {
                ImGui::Text(
                    "beat %d,%d to %d,%d",
                    spawn.patrol->from.x,
                    spawn.patrol->from.y,
                    spawn.patrol->to.x,
                    spawn.patrol->to.y);

                std::optional<std::pair<glm::vec2, glm::vec2>> beat = level.patrolFor(spawn);
                if (npc && beat &&
                    !canPatrolBetween(
                        level.graphFor(npc->getNavigationProfile()), beat->first, beat->second))
                    ImGui::TextColored(
                        ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "it cannot get back from there");
            }
            else
                ImGui::TextDisabled("no beat");

            if (npc)
                ImGui::Text("at %.0f,%.0f", npc->getPosition().x, npc->getPosition().y);
            else
                ImGui::TextDisabled("not spawned");

            ImGui::TreePop();
        }

        ImGui::PopID();
    }
}
