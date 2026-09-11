#include <vector>
#include "pickups/collecting.hpp"
#include "pickups/pickup.hpp"
#include "physics/aabb.hpp"

std::vector<Pickup> takeWhatTouches(std::vector<Pickup> &pickups, const AABB &reach)
{
    std::vector<Pickup> taken;
    for (Pickup &pickup : pickups)
    {
        if (!pickup.stillThere() || !pickup.getAABB().intersects(reach))
            continue;

        pickup.taken();
        taken.push_back(pickup);
    }

    return taken;
}
