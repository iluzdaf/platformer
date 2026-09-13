#pragma once

#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "events/event.hpp"
#include "actor/appearance.hpp"
#include "assets/sheet_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/abilities/abilities.hpp"
#include "actor/abilities/ability_states.hpp"
#include "actor/observed.hpp"
#include "actor/perceived.hpp"
#include "actor/actor_behavior.hpp"
#include "animations/animator.hpp"
#include "physics/physics_body.hpp"
#include <limits>
#include "navigation/navigation_profile.hpp"
#include "actor/actor_facts.hpp"
#include "actor/behaviors/senses_data.hpp"
#include "actor/behaviors/patrol_data.hpp"
#include "conditions/asked.hpp"
#include "actor/declared_facts.hpp"
#include "actor/fading_facts.hpp"
#include "conditions/facts.hpp"
#include "game/noise.hpp"
#include "combat/health.hpp"
#include "combat/hurting.hpp"

class StateScript;
class TileMap;
class Level;
class NavigationGraph;
struct Hit;

class Actor
{
public:
    virtual ~Actor() = default;
    void beginFrame();
    void fixedUpdate(float deltaTime, const Level &level, const Perceived &perceived = {});
    const Appearance &appearance() const;
    const AbilityStates &abilityStates() const;
    const Observed &observed() const;
    const PhysicsBody &body() const;
    const NavigationProfile &profile() const;
    std::string_view stateName() const;
    const SheetData &drawnFrom() const;
    std::optional<int> currentNodeId() const;
    std::optional<int> targetNodeId() const;
    glm::vec2 feet() const;
    void standAt(const glm::vec2 &feet);
    const Health &health() const;
    bool alive() const;
    bool takeHit(const Hit &hit);
    std::optional<Hurting> hurting() const;
    bool strike(Actor &target);
    Event<Actor> onHurt, onDeath;
    Event<Actor, const std::string &> onCue;
    Event<Actor, float> onTick;
    Event<Actor, const Noise &> onNoise;

    const FactsData &facts() const;
    const FadingFacts &saidLately() const;
    const Asked &fact(const std::string &name) const;
    void fact(const std::string &name, const Asked &value);
    void event(const std::string &name, const Asked &value);
    std::optional<glm::vec2> threatFeet() const;
    float distanceTo(glm::vec2 at) const;
    bool onSameSurfaceAs(glm::vec2 at) const;
    bool corneredBy(glm::vec2 at) const;
    bool onGround() const;
    void walks(const NavigationGraph &navigationGraph);

protected:
    explicit Actor(const ActorData &data);
    void setBehavior(std::unique_ptr<ActorBehavior> newBehavior);
    void declare(const FactsData &facts);
    void setSenses(const SensesData &newSenses);
    void setBeat(const std::optional<PatrolData> &newBeat);
    void scriptBehaviorWith(StateScript *script);

private:
    const NavigationGraph &graphWalked() const;
    ActorFacts factsNow() const;
    float howFarItFell();
    DeclaredFacts declaredFacts;
    SensesData senses;
    std::optional<PatrolData> beat;
    const NavigationGraph *walking = nullptr;
    float highestSinceTheGround = std::numeric_limits<float>::max();
    float fallFromHeightThreshold = 0.0f;
    std::optional<glm::vec2> threat;
    Abilities abilities;
    AbilityStates states;
    std::vector<const Actor *> struckThisSwing;
    Observed observations;
    PhysicsBody physicsBody;
    std::optional<Animator> animator;
    Appearance shown;
    SheetData sheet;
    NavigationProfile navigationProfile;
    Health hp;
    std::unique_ptr<ActorBehavior> behavior;
};
