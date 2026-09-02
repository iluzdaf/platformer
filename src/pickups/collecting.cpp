#include <algorithm>
#include <iterator>
#include <vector>
#include "pickups/collecting.hpp"
#include "pickups/pickup.hpp"
#include "physics/aabb.hpp"

std::vector<Pickup> takeWhatTouches(std::vector<Pickup> &pickups, const AABB &reach)
{
    std::vector<Pickup> taken;

    auto touched = std::stable_partition(
        pickups.begin(),
        pickups.end(),
        [&reach](const Pickup &pickup) { return !pickup.getAABB().intersects(reach); });

    taken.assign(std::make_move_iterator(touched), std::make_move_iterator(pickups.end()));
    pickups.erase(touched, pickups.end());

    return taken;
}
