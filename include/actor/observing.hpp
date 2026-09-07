#pragma once

#include "actor/actor_contact_state.hpp"

class PhysicsBody;
class TileMap;

ActorContactState contactsForANewFrame(ActorContactState contacts);
ActorContactState contactsAfterStep(
    const ActorContactState &previous,
    const PhysicsBody &physicsBody,
    const TileMap &tileMap);
