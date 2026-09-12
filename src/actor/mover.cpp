#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/mover.hpp"
#include "actor/abilities/abilities_data.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/observing.hpp"
#include "combat/hit.hpp"
#include "input/input_intentions.hpp"
#include "physics/physics_body.hpp"
#include "physics/physics_body_data.hpp"
#include "tile_map/tile_map.hpp"

Mover::Mover(const AbilitiesData &abilitiesData, const PhysicsBodyData &physicsBodyData)
    : abilities(abilitiesData), physicsBody(physicsBodyData)
{
}

void Mover::beginFrame()
{
    observations.contacts = contactsForANewFrame(observations.contacts);
}

void Mover::lookAround(const TileMap &tileMap)
{
    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
}

void Mover::step(float deltaTime, const InputIntentions &inputIntentions, const TileMap &tileMap)
{
    glm::vec2 velocity = abilities.decide(deltaTime, inputIntentions, observations, abilityStates);
    observations.hits.clear();

    physicsBody.setVelocity(velocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
    observations.previousVelocity = observations.velocity;
    observations.velocity = physicsBody.velocity();
    observations.fell = howFarItFell();

    if (!abilityStates.knockback.active)
        observations.facingLeft =
            observations.velocity.x > 0
                ? false
                : (observations.velocity.x < 0 ? true : observations.facingLeft);
}

float Mover::howFarItFell()
{
    if (!observations.contacts.onGround)
    {
        highestSinceTheGround = std::min(highestSinceTheGround, feet().y);
        return 0.0f;
    }

    float fell = observations.contacts.wasOnGround ? 0.0f : feet().y - highestSinceTheGround;
    highestSinceTheGround = feet().y;

    return fell;
}

void Mover::standAt(glm::vec2 feet)
{
    physicsBody.setPosition(feet - physicsBody.bottomCenterOffset());
}

void Mover::observe(const Hit &hit)
{
    observations.hits.push_back(hit);
}

void Mover::observeAlive(bool alive)
{
    observations.alive = alive;
}

glm::vec2 Mover::feet() const
{
    return physicsBody.aabb().bottomCenter();
}

const PhysicsBody &Mover::body() const
{
    return physicsBody;
}

const AbilityStates &Mover::states() const
{
    return abilityStates;
}

const Observed &Mover::observed() const
{
    return observations;
}
