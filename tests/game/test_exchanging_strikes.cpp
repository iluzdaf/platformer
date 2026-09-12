#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include "actor/abilities/charge_ability_data.hpp"
#include "actor/abilities/charge_ability_state.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/abilities/swing_ability_state.hpp"
#include "actor/ability_states.hpp"
#include "actor/actor_state.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "animations/animation_rule_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "combat/health.hpp"
#include "combat/health_data.hpp"
#include "combat/hit.hpp"
#include "combat/hurting.hpp"
#include "conditions/asked.hpp"
#include "game/exchanging_strikes.hpp"
#include "game/level.hpp"
#include "helpers/actors.hpp"
#include "helpers/floor_level.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/palettes.hpp"
#include "helpers/player_fixtures.hpp"
#include "helpers/rules.hpp"
#include "helpers/tile_positions.hpp"
#include "input/input_intentions.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "player/player.hpp"
#include "player/player_data.hpp"
#include "timing/fixed_time_step.hpp"

namespace
{
    constexpr glm::ivec2 PlayerTile{4, FloorLevelStanding};

    PlayerData aSwordsman()
    {
        PlayerData playerData = playerDataWithEveryAbility();
        SwingAbilityData &swing = *playerData.actorData.motionData.swingAbilityData;
        swing.windupDuration = 0.05f;
        swing.strikeDuration = 0.2f;
        swing.recoveryDuration = 0.05f;
        return playerData;
    }

    NpcData aVillagerWith(int points)
    {
        NpcData rat = setupNpcData();
        rat.actorData.healthData = HealthData{points, 0.0f};
        AnimatorData &animations = rat.actorData.animationData.emplace();
        animations.startClip = "idle";
        animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
        animations.clips["dead"] = FrameAnimationData({7}, 1.0f);
        AnimationWhenData dead;
        dead["alive"] = false;
        animations.rules = {{"dead", dead}};
        return rat;
    }

    struct Duel
    {
        Duel(int ratPoints, glm::ivec2 ratTile)
            : playerData(aSwordsman()), level(
                                            aFloorLevelPlacing({spawnAt("rat", ratTile)}),
                                            theOnlyPalette(aPaletteWithASolidTile()),
                                            playerData,
                                            {{"rat", aVillagerWith(ratPoints)}},
                                            {}),
              player(playerData, input)
        {
            player.standAt(feetOf(PlayerTile));
        }

        Npc &rat()
        {
            return *level.getNpcs().front();
        }

        void swingFor(float seconds)
        {
            InputIntentions attack;
            attack.attack = std::string(SwingAttack);
            input.set(attack);
            runFor(player, level, seconds, timestepper);
            input.set(InputIntentions{});
        }

        ScriptedIntentions input;
        PlayerData playerData;
        Level level;
        Player player;
        FixedTimeStep timestepper;
    };

    constexpr glm::ivec2 Here{4, 5};

    std::unique_ptr<Npc> anNpcThatBites(int damage, glm::ivec2 tile = Here)
    {
        NpcData biting = setupNpcData();
        biting.contactDamage = damage;
        return std::make_unique<Npc>(spawnAt("biter", tile), biting);
    }

    std::vector<std::unique_ptr<Npc>> oneNpcThatBites(int damage, glm::ivec2 tile = Here)
    {
        std::vector<std::unique_ptr<Npc>> npcs;
        npcs.push_back(anNpcThatBites(damage, tile));
        return npcs;
    }
}

TEST_CASE("A swing in front of the player costs the npc the swing's damage", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    REQUIRE(duel.player.abilityStates().swing.striking());

    exchangeStrikes(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 2);
    REQUIRE(duel.rat().health().lastHit()->direction == glm::vec2(1.0f, 0.0f));
}

TEST_CASE("A swing lands once, however long the npc stays in reach", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);

    exchangeStrikes(duel.player, duel.level.getNpcs());
    duel.swingFor(0.05f);
    REQUIRE(duel.player.abilityStates().swing.striking());
    exchangeStrikes(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 2);
}

TEST_CASE("The next swing lands again on an npc the last one struck", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    exchangeStrikes(duel.player, duel.level.getNpcs());
    runFor(duel.player, duel.level, 0.25f, duel.timestepper);
    REQUIRE_FALSE(duel.player.abilityStates().swing.swinging());

    duel.swingFor(0.1f);
    REQUIRE(duel.player.abilityStates().swing.striking());
    exchangeStrikes(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 1);
}

TEST_CASE("A swing behind the player misses", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile - glm::ivec2(1, 0));
    duel.swingFor(0.1f);

    exchangeStrikes(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 3);
}

TEST_CASE("Between swings, nothing lands", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    runFor(duel.player, duel.level, 0.1f, duel.timestepper);
    REQUIRE_FALSE(duel.player.hurting());

    exchangeStrikes(duel.player, duel.level.getNpcs());

    REQUIRE(duel.rat().health().points() == 3);
}

TEST_CASE("A swing reaches out from the collider on the side it faces", "[ExchangingStrikes]")
{
    Duel duel(3, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    AABB collider = duel.player.body().aabb();

    AABB reach = duel.player.hurting().value().box;

    REQUIRE(reach.left() == collider.right());
    REQUIRE(reach.size == duel.playerData.actorData.motionData.swingAbilityData->reach);
    REQUIRE(reach.center().y == collider.center().y);
}

TEST_CASE("A corpse stops deciding, shows it, and takes no more hits", "[ExchangingStrikes]")
{
    Duel duel(1, PlayerTile + glm::ivec2(1, 0));
    duel.swingFor(0.1f);
    exchangeStrikes(duel.player, duel.level.getNpcs());
    REQUIRE_FALSE(duel.rat().alive());

    duel.rat().beginFrame();
    duel.rat().fixedUpdate(0.01f, duel.level, duel.player.feet());

    REQUIRE(duel.rat().stateName() == std::string_view{});
    REQUIRE(duel.rat().state().currentAnimation == "dead");
    REQUIRE_FALSE(duel.player.strike(duel.rat()));
}

TEST_CASE("Standing in an npc that bites costs its damage", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));

    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(2);

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 1);
}

TEST_CASE("An npc with no bite is safe to stand in, and hurts nothing", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(0);
    bool hurt = false;
    player.onHurt.connect([&] { hurt = true; });

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 3);
    REQUIRE_FALSE(hurt);
    REQUIRE_FALSE(player.health().invulnerable());
}

TEST_CASE(
    "A creature's hurt box is its body while it bites, and nothing while it is safe",
    "[ExchangingStrikes]")
{
    std::vector<std::unique_ptr<Npc>> biting = oneNpcThatBites(2);
    std::vector<std::unique_ptr<Npc>> safe = oneNpcThatBites(0);

    REQUIRE(biting.front()->hurting().has_value());
    REQUIRE(biting.front()->hurting()->box.position == biting.front()->body().aabb().position);
    REQUIRE(biting.front()->hurting()->box.size == biting.front()->body().aabb().size);
    REQUIRE(biting.front()->hurting()->damage == 2);
    REQUIRE_FALSE(safe.front()->hurting().has_value());
}

TEST_CASE("A dead creature hurts with nothing", "[ExchangingStrikes]")
{
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(2);
    npcs.front()->takeHit(Hit{99, glm::vec2(1.0f, 0.0f), false});

    REQUIRE_FALSE(npcs.front()->alive());
    REQUIRE_FALSE(npcs.front()->hurting().has_value());
}

TEST_CASE("An npc a tile away does not reach", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1, Here + glm::ivec2(2, 0));

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A dead npc bites nobody", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    npcs.front()->takeHit(lethalHit());

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A bite pushes away from the npc", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    player.standAt(npcs.front()->feet() + glm::vec2(2.0f, 0.0f));
    glm::vec2 pushed(0.0f);
    player.onHurt.connect([&] { pushed = player.health().lastHit()->direction; });

    exchangeStrikes(player, npcs);

    REQUIRE(pushed.x > 0.0f);
}

TEST_CASE("A bite lands once per invulnerable window", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);

    exchangeStrikes(player, npcs);
    exchangeStrikes(player, npcs);
    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE("Two npcs biting the player at once cost it one bite a tick", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs;
    npcs.push_back(anNpcThatBites(1));
    npcs.push_back(anNpcThatBites(1));

    exchangeStrikes(player, npcs);
    REQUIRE(player.health().points() == 2);

    exchangeStrikes(player, npcs);
    REQUIRE(player.health().points() == 1);
}

TEST_CASE("An npc that misses does not stop the next one biting", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs;
    npcs.push_back(anNpcThatBites(1, Here + glm::ivec2(2, 0)));
    npcs.push_back(anNpcThatBites(1));

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE(
    "A creature bites while its pounce is in the air, and not on the ground",
    "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(SpawnTile));

    NpcData pouncer = setupNpcData();
    pouncer.actorData.motionData.pounceAbilityData = PounceAbilityData{};
    BehaviorStateData pouncing;
    pouncing.name = "pounce";
    pouncing.does = AttackBehaviorData{std::string(PounceAttack)};
    pouncer.stateMachineBehaviorData->states = {pouncing};
    std::vector<std::unique_ptr<Npc>> npcs;
    npcs.push_back(std::make_unique<Npc>(spawnAt("pouncer", SpawnTile), pouncer));
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 0.0f),
        {{"pouncer", pouncer}},
        {});
    for (int settle = 0; settle < 30; ++settle)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level);
    }

    REQUIRE_FALSE(npcs.front()->hurting().has_value());
    exchangeStrikes(player, npcs);
    REQUIRE(player.health().points() == 3);

    for (int step = 0; step < 3; ++step)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level, player.feet() + glm::vec2(4.0f, 0.0f));
    }
    REQUIRE(npcs.front()->abilityStates().pounce.active);
    REQUIRE(npcs.front()->hurting().has_value());
    REQUIRE(npcs.front()->hurting()->box.position == npcs.front()->body().aabb().position);
    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE("A creature bites while it charges, and not while it stands", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(SpawnTile));

    NpcData charger = setupNpcData();
    charger.actorData.motionData.chargeAbilityData = ChargeAbilityData{};
    BehaviorStateData charging;
    charging.name = "charge";
    charging.does = AttackBehaviorData{std::string(ChargeAttack)};
    charger.stateMachineBehaviorData->states = {charging};
    std::vector<std::unique_ptr<Npc>> npcs;
    npcs.push_back(std::make_unique<Npc>(spawnAt("charger", SpawnTile), charger));
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 0.0f),
        {{"charger", charger}},
        {});
    for (int settle = 0; settle < 30; ++settle)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level);
    }

    REQUIRE_FALSE(npcs.front()->hurting().has_value());
    exchangeStrikes(player, npcs);
    REQUIRE(player.health().points() == 3);

    for (int step = 0; step < 2; ++step)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level, player.feet() + glm::vec2(4.0f, 0.0f));
    }
    REQUIRE(npcs.front()->abilityStates().charge.active);
    REQUIRE(npcs.front()->hurting().has_value());
    REQUIRE(npcs.front()->hurting()->box.position == npcs.front()->body().aabb().position);
    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE("A creature with a swing strikes the player with it", "[ExchangingStrikes]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(SpawnTile + glm::ivec2(1, 0)));

    NpcData swinger = setupNpcData();

    AnimatorData &animations = swinger.actorData.animationData.emplace();
    animations.startClip = "idle";
    animations.clips["idle"] = FrameAnimationData({0}, 1.0f);
    swinger.actorData.motionData.swingAbilityData = SwingAbilityData{};
    animations.clips["attack"] = anAttackClip();
    AnimationWhenData whileSwinging;
    whileSwinging["swinging"] = true;
    animations.rules = {AnimationRuleData{"attack", whileSwinging}, idleRule()};

    BehaviorStateData swinging;
    swinging.name = "swing";
    swinging.does = AttackBehaviorData{std::string(SwingAttack)};
    swinger.stateMachineBehaviorData->states = {swinging};
    std::vector<std::unique_ptr<Npc>> npcs;
    npcs.push_back(std::make_unique<Npc>(spawnAt("swinger", SpawnTile), swinger));
    Level level(
        aFloorLevelPlacing({}),
        theOnlyPalette(aPaletteWithASolidTile()),
        playerDataWithHealth(3, 0.0f),
        {{"swinger", swinger}},
        {});

    for (int step = 0; step < 60 && !npcs.front()->hurting(); ++step)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level, player.feet());
    }
    REQUIRE(npcs.front()->hurting().has_value());
    REQUIRE(npcs.front()->hurting()->box.left() == npcs.front()->body().aabb().right());

    exchangeStrikes(player, npcs);

    REQUIRE(player.health().points() == 2);
}
