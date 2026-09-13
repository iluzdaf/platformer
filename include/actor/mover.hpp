#pragma once

#include <limits>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "physics/physics_body.hpp"

struct AbilitiesData;
struct PhysicsBodyData;
struct InputIntentions;
struct Hit;
class TileMap;

class Mover
{
public:
    Mover(const AbilitiesData &abilitiesData, const PhysicsBodyData &physicsBodyData);

    void beginFrame();
    void lookAround(const TileMap &tileMap);
    void step(float deltaTime, const InputIntentions &inputIntentions, const TileMap &tileMap);
    void standAt(glm::vec2 feet);
    void observe(const Hit &hit);
    void observeAlive(bool alive);

    glm::vec2 feet() const;
    const PhysicsBody &body() const;
    const AbilityStates &states() const;
    const Observed &observed() const;

private:
    float howFarItFell();
    Abilities abilities;
    AbilityStates abilityStates;
    Observed observations;
    PhysicsBody physicsBody;
    float highestSinceTheGround = std::numeric_limits<float>::max();
};
