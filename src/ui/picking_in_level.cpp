#include <cstddef>
#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "ui/picking_in_level.hpp"
#include "ui/actors_in_level.hpp"
#include "game/level.hpp"
#include "npc/npc.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "pickups/pickup.hpp"

ActorShown whatIsAt(const Level &level, const AABB &playerBox, glm::vec2 at)
{
    if (playerBox.covers(at))
        return ActorShown{ActorShown::What::Player, 0};

    const std::vector<std::unique_ptr<Npc>> &npcs = level.getNpcs();
    for (std::size_t behind = npcs.size(); behind > 0; --behind)
    {
        std::size_t index = behind - 1;
        if (npcs[index]->body().aabb().covers(at))
            return ActorShown{ActorShown::What::Npc, index};
    }

    const std::vector<Pickup> &pickups = level.getPickups();
    for (std::size_t behind = pickups.size(); behind > 0; --behind)
    {
        std::size_t index = behind - 1;
        if (pickups[index].stillThere() && pickups[index].getAABB().covers(at))
            return ActorShown{ActorShown::What::Pickup, index};
    }

    return ActorShown{};
}
