#include <algorithm>
#include <stdexcept>
#include <optional>
#include <string>
#include <string_view>
#include "actor/actor.hpp"
#include "actor/fading_facts.hpp"
#include "actor/actor_data.hpp"
#include "combat/hit.hpp"
#include "combat/hurting.hpp"
#include "combat/strike.hpp"
#include "actor/hurting_from.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "physics/aabb.hpp"
#include "actor/observing.hpp"
#include "actor/cues.hpp"
#include "actor/facing.hpp"
#include "actor/observed.hpp"
#include "actor/perceived.hpp"
#include "actor/abilities/ability_states.hpp"
#include "animations/frame_animation.hpp"
#include "animations/animator.hpp"
#include "animations/animator_facts.hpp"
#include "actor/actor_fact_rows.hpp"
#include "actor/actor_facts.hpp"
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

Actor::Actor(const ActorData &data)
    : fallFromHeightThreshold(data.fallFromHeightThreshold), abilities(data.abilities),
      physicsBody(data.physicsBodyData), navigationProfile(buildNavigationProfile(data)),
      hp(data.healthData)
{
    if (data.animationData)
        animator.emplace(*data.animationData);

    shown.currentAnimation = animator ? animator->state() : std::string();
    shown.currentFrame = animator ? animator->playing().frame() : 0;

    sheet = data.sheet;
    shown.size = drawnSizeOf(data);
    if (shown.size.x <= 0.0f || shown.size.y <= 0.0f)
        throw std::runtime_error("An actor drawn as nothing is one nobody can see");
}

void Actor::beginFrame()
{
    observations.contacts = contactsForANewFrame(observations.contacts);
}

void Actor::fixedUpdate(float deltaTime, const Level &level, const Perceived &perceived)
{
    const TileMap &tileMap = level.getTileMap();
    hp.update(deltaTime);
    observations.alive = hp.alive();
    walks(level.graphFor(navigationProfile));
    threat = perceived.threatFeet;
    for (const Noise &noise : perceived.noises)
        onNoise(noise);
    onTick(deltaTime);

    InputIntentions inputIntentions =
        behavior ? behavior->decide(deltaTime, factsNow()) : InputIntentions();

    glm::vec2 velocity = abilities.decide(deltaTime, inputIntentions, observations, states);
    observations.hits.clear();
    if (!states.swing.striking())
        struckThisSwing.clear();

    physicsBody.setVelocity(velocity);
    physicsBody.stepPhysics(deltaTime, tileMap);

    observations.contacts = contactsAfterStep(observations.contacts, physicsBody, tileMap);
    observations.previousVelocity = observations.velocity;
    observations.velocity = physicsBody.velocity();
    observations.fell = howFarItFell();
    declaredFacts.fade(deltaTime);

    if (animator)
        animator->animate(deltaTime, factsNow());

    observations.facingLeft =
        facingLeftAfter(observations.facingLeft, observations.velocity.x, states.knockback.active);

    if (animator)
    {
        shown.currentFrame = animator->playing().frame();
        shown.currentAnimation = animator->state();
        for (const std::string &cue : animator->takeCues())
            onCue(cue);
    }

    for (std::string_view cue : cuesOf(states, observations, fallFromHeightThreshold))
        onCue(std::string(cue));

    declaredFacts.forgetTheTick();
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

void Actor::declare(const FactsData &facts)
{
    declaredFacts = DeclaredFacts(facts);
}

void Actor::setSenses(const SensesData &newSenses)
{
    senses = newSenses;
}

void Actor::scriptBehaviorWith(StateScript *script)
{
    if (behavior)
        behavior->scriptWith(script);
}

const FactsData &Actor::facts() const
{
    return declaredFacts.all();
}

const Asked &Actor::fact(const std::string &name) const
{
    return declaredFacts.fact(name);
}

void Actor::fact(const std::string &name, const Asked &value)
{
    declaredFacts.fact(name, value);
}

void Actor::event(const std::string &name, const Asked &value)
{
    declaredFacts.event(name, value);
}

const FadingFacts &Actor::saidLately() const
{
    return declaredFacts.saidLately();
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

const Appearance &Actor::appearance() const
{
    return shown;
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
        onHurt();
    }
    else
        onDeath();

    return true;
}

std::optional<Hurting> Actor::hurting() const
{
    if (!alive())
        return std::nullopt;

    return hurtingFrom(states, physicsBody.aabb());
}

bool Actor::strike(Actor &target)
{
    std::optional<Hurting> hurting = this->hurting();
    if (!hurting || &target == this)
        return false;

    bool swinging = states.swing.striking();
    if (swinging && std::ranges::find(struckThisSwing, &target) != struckThisSwing.end())
        return false;

    std::optional<Hit> hit = hitFrom(*hurting, feet(), target.body().touchBox(), target.feet());
    if (!hit || !target.takeHit(*hit))
        return false;

    if (swinging)
        struckThisSwing.push_back(&target);
    return true;
}

void Actor::setBehavior(std::unique_ptr<ActorBehavior> newBehavior)
{
    behavior = std::move(newBehavior);
}

ActorFacts Actor::factsNow() const
{
    ActorFacts facts{
        graphWalked(),
        feet(),
        physicsBody.colliderSize(),
        threat,
        observations.contacts,
        &declaredFacts.all(),
        &states,
        &senses};
    facts.velocity = observations.velocity;
    facts.alive = observations.alive;
    facts.inState = stateName();
    return facts;
}
