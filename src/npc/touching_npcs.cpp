#include <memory>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "npc/touching_npcs.hpp"
#include "actor/hit.hpp"
#include "npc/npc.hpp"
#include "physics/aabb.hpp"
#include "physics/physics_body.hpp"
#include "player/player.hpp"

void touchNpcs(Player &player, const std::vector<std::unique_ptr<Npc>> &npcs)
{
    AABB touching = player.body().touchBox();
    for (const std::unique_ptr<Npc> &npc : npcs)
    {
        if (npc->contactDamage() <= 0 || !npc->alive() || !touching.intersects(npc->body().aabb()))
            continue;

        float away = player.feet().x < npc->feet().x ? -1.0f : 1.0f;
        if (player.takeHit(Hit{npc->contactDamage(), glm::vec2(away, 0.0f), false}))
            return;
    }
}
