#include <algorithm>
#include <stdexcept>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/fading_facts.hpp"
#include "actor/actor_data.hpp"
#include "actor/hit.hpp"
#include "actor/hurting.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "physics/aabb.hpp"
#include "actor/observing.hpp"
#include "actor/observed.hpp"
#include "actor/ability_states.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animator.hpp"
#include "animations/animator_facts.hpp"
#include "actor/behaviors/behavior_facts.hpp"
#include "conditions/fact_rows.hpp"
#include "actor/actor_behavior_context.hpp"
#include "navigation/navigation_graph.hpp"
#include "navigation/navigation_place.hpp"
#include "navigation/navigation_profile.hpp"
#include "conditions/asked.hpp"
#include "conditions/facts.hpp"
#include "navigation/navigation_profile_builder.hpp"
#include "input/input_intentions.hpp"
#include "tile_map/tile_map.hpp"
#include "game/level.hpp"
#include "game/noise.hpp"
#include <glm/geometric.hpp>
#include <optional>
#include <memory>
#include <utility>
#include <vector>

namespace
{
    constexpr float SaidLingersFor = 0.5f;
}

Actor::Actor(const ActorData &data)
    : abilities(data.motionData), physicsBody(data.physicsBodyData),
      navigationProfile(buildNavigationProfile(data)), hp(data.healthData)
{
    if (data.animationData)
        animator.emplace(*data.animationData);

    actorState.currentAnimation = animator ? animator->state() : std::string();
    actorState.currentFrame = animator ? animator->playing().frame() : 0;

    sheet = data.sheet;
    actorState.size = drawnSizeOf(data);
    if (actorState.size.x <= 0.0f || actorState.size.y <= 0.0f)
        throw std::runtime_error("An actor drawn as nothing is one nobody can see");
}

void Actor::postFixedUpdate()
{
}

void Actor::beginFrame()
{
    observations.contacts = contactsForANewFrame(observations.contacts);
}

void Actor::fixedUpdate(
    float deltaTime,
    const Level &level,
    std::optional<glm::vec2> threatFeet,
    std::span<const Noise> noises)
{
    const TileMap &tileMap = level.getTileMap();
    hp.update(deltaTime);
    observations.alive = hp.alive();
    walks(level.graphFor(navigationProfile));
    threat = threatFeet;
    for (const Noise &noise : noises)
        onNoise(noise);
    onTick(deltaTime);

    ActorBehaviorContext context = behaviorContext(*walking);
    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, context) : InputIntentions();

    glm::vec2 velocity = abilities.decide(deltaTime, inputIntentions, observations, states);
    observations.hits.clear();

    physicsBody.setVelocity(velocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
    observations.previousVelocity = observations.velocity;
    observations.velocity = physicsBody.velocity();
    observations.fell = howFarItFell();
    lately.update(deltaTime);

    if (animator)
        animator->animate(deltaTime, states, observations, stateName());

    if (!states.knockback.active)
        actorState.facingLeft = observations.velocity.x > 0
                                    ? false
                                    : (observations.velocity.x < 0 ? true : actorState.facingLeft);
    observations.facingLeft = actorState.facingLeft;

    if (animator)
    {
        actorState.currentFrame = animator->playing().frame();
        actorState.currentAnimation = animator->state();
        for (const std::string &cue : animator->takeCues())
            onCue(cue);
    }

    forgetTheTick();
}

float Actor::howFarItFell()
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

void Actor::forgetTheTick()
{
    for (const std::string &name : saidForTheTick)
        known[name] = declared.at(name);

    saidForTheTick.clear();
}

void Actor::declare(const FactsData &facts)
{
    for (const auto &[name, value] : facts)
        if (rowNamed(behaviorRows(), name))
            throw std::runtime_error(
                "\"" + name + "\" is a fact the engine answers, and cannot be declared");

    declared = facts;
    known = facts;
    saidForTheTick.clear();
}

const FactsData &Actor::facts() const
{
    return known;
}

const Asked &Actor::fact(const std::string &name) const
{
    auto found = known.find(name);
    if (found == known.end())
        throw std::runtime_error("\"" + name + "\" is not a declared fact");

    return found->second;
}

void Actor::say(const std::string &name, const Asked &value)
{
    if (std::optional<std::string> why = whyNotDeclared(declared, name, value))
        throw std::runtime_error(*why);

    known[name] = value;
}

void Actor::fact(const std::string &name, const Asked &value)
{
    say(name, value);
}

void Actor::event(const std::string &name, const Asked &value)
{
    say(name, value);
    saidForTheTick.push_back(name);
    lately.said(name, value, SaidLingersFor);
}

const FadingFacts &Actor::saidLately() const
{
    return lately;
}

void Actor::walks(const NavigationGraph &navigationGraph)
{
    walking = &navigationGraph;
}

const NavigationGraph &Actor::graphWalked() const
{
    if (!walking)
        throw std::runtime_error("Nothing can be asked about the ground before the first tick");

    return *walking;
}

std::optional<glm::vec2> Actor::threatFeet() const
{
    return threat;
}

float Actor::distanceTo(glm::vec2 at) const
{
    return glm::distance(feet(), at);
}

bool Actor::onSameSurfaceAs(glm::vec2 at) const
{
    return onTheSameRun(graphWalked(), feet(), at);
}

bool Actor::corneredBy(glm::vec2 at) const
{
    return ::corneredBy(graphWalked(), feet(), at, physicsBody.colliderSize().x);
}

bool Actor::onGround() const
{
    return observations.contacts.onGround;
}

const SheetData &Actor::drawnFrom() const
{
    return sheet;
}

const ActorState &Actor::state() const
{
    return actorState;
}

const AbilityStates &Actor::abilityStates() const
{
    return states;
}

const Observed &Actor::observed() const
{
    return observations;
}

const PhysicsBody &Actor::body() const
{
    return physicsBody;
}

const NavigationProfile &Actor::profile() const
{
    return navigationProfile;
}

std::string_view Actor::stateName() const
{
    return behavior ? behavior->getStateName() : std::string_view{};
}

std::optional<int> Actor::currentNodeId() const
{
    return behavior ? behavior->getCurrentNodeId() : std::nullopt;
}

std::optional<int> Actor::targetNodeId() const
{
    return behavior ? behavior->getTargetNodeId() : std::nullopt;
}

glm::vec2 Actor::feet() const
{
    return physicsBody.aabb().bottomCenter();
}

void Actor::standAt(const glm::vec2 &newFeet)
{
    physicsBody.setPosition(newFeet - physicsBody.bottomCenterOffset());

    if (behavior)
        behavior->reset();
}

const Health &Actor::health() const
{
    return hp;
}

bool Actor::alive() const
{
    return hp.alive();
}

bool Actor::takeHit(const Hit &hit)
{
    if (!hp.takeHit(hit))
        return false;

    if (hp.alive())
    {
        observations.hits.push_back(hit);
        hurt();
        onHurt();
    }
    else
    {
        died();
        onDeath();
    }

    return true;
}

std::optional<AABB> Actor::swingBox() const
{
    const SwingAbilityState &swing = states.swing;
    if (!swing.striking())
        return std::nullopt;

    AABB collider = physicsBody.aabb();
    float x = swing.direction < 0.0f ? collider.left() - swing.reach.x : collider.right();
    return AABB{glm::vec2(x, collider.center().y - swing.reach.y * 0.5f), swing.reach};
}

std::optional<Hurting> Actor::hurting() const
{
    if (!alive())
        return std::nullopt;

    const SwingAbilityState &swing = states.swing;
    if (std::optional<AABB> reach = swingBox())
        return Hurting{*reach, swing.damage, swing.direction};

    const PounceAbilityState &pounce = states.pounce;
    if (pounce.active)
        return Hurting{physicsBody.aabb(), pounce.damage, pounce.direction};

    const ChargeAbilityState &charge = states.charge;
    if (charge.active)
        return Hurting{physicsBody.aabb(), charge.damage, charge.direction};

    return std::nullopt;
}

bool Actor::strike(Actor &target)
{
    std::optional<Hurting> hurting = this->hurting();
    if (!hurting || &target == this)
        return false;

    SwingAbilityState &swing = states.swing;
    if (swing.striking() && std::ranges::find(swing.struck, &target) != swing.struck.end())
        return false;

    if (!hurting->box.intersects(target.body().touchBox()))
        return false;

    float away = target.feet().x < feet().x ? -1.0f : 1.0f;
    float direction = hurting->direction != 0.0f ? hurting->direction : away;
    if (!target.takeHit(Hit{hurting->damage, glm::vec2(direction, 0.0f), false}))
        return false;

    if (swing.striking())
        swing.struck.push_back(&target);
    return true;
}

void Actor::hurt()
{
}

void Actor::died()
{
}

void Actor::setBehavior(std::unique_ptr<ActorBehavior> newBehavior)
{
    behavior = std::move(newBehavior);
}

ActorBehaviorContext Actor::behaviorContext(const NavigationGraph &navigationGraph) const
{
    return ActorBehaviorContext{
        navigationGraph,
        feet(),
        physicsBody.colliderSize(),
        threat,
        observations.contacts,
        &known,
        &states};
}
