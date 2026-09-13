#include <stdexcept>
#include <string>
#include "actor/behaviors/attack_behavior.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "input/input_intentions.hpp"

AttackBehavior::AttackBehavior(const AttackBehaviorData &data) : with(data.with)
{
    if (with.empty())
        throw std::runtime_error("An attack must say what it is made with");
}

void AttackBehavior::reset()
{
    asked = false;
}

InputIntentions AttackBehavior::decide(float, const ActorFacts &context)
{
    InputIntentions intentions;
    if (!context.threatFeet || asked || !context.contacts.onGround)
        return intentions;

    float towards = context.threatFeet->x - context.feet.x;
    intentions.direction.x = towards < 0.0f ? -1.0f : 1.0f;
    intentions.attack = with;
    asked = true;
    return intentions;
}
