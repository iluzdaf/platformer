#include <memory>
#include <optional>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <string>
#include "actor/decided.hpp"
#include "actor/abilities/pounce_ability_state.hpp"
#include "actor/abilities/pounce_ability_data.hpp"
#include "actor/abilities/swing_ability_data.hpp"
#include "actor/hurting.hpp"
#include "helpers/palettes.hpp"
#include "game/level.hpp"
#include "actor/behaviors/state_machine_behavior_data.hpp"
#include "actor/behaviors/attack_behavior_data.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "actor/health.hpp"
#include "actor/hit.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "animations/animator_data.hpp"
#include "npc/striking_player.hpp"
#include "player/player.hpp"

namespace
{
    constexpr glm::ivec2 Here{4, 5};

    std::vector<std::unique_ptr<Npc>> oneNpcThatBites(int damage, glm::ivec2 tile = Here)
    {
        NpcData biting = setupNpcData();
        biting.contactDamage = damage;
        std::vector<std::unique_ptr<Npc>> npcs;
        npcs.push_back(std::make_unique<Npc>(spawnAt("biter", tile), biting));
        return npcs;
    }

}

TEST_CASE("Standing in an npc that bites costs its damage", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(2);

    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 1);
}

TEST_CASE("An npc with no bite is safe to stand in, and hurts nothing", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(0);
    bool hurt = false;
    player.onHurt.connect([&] { hurt = true; });

    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 3);
    REQUIRE_FALSE(hurt);
    REQUIRE_FALSE(player.health().invulnerable());
}

TEST_CASE(
    "A creature's hurt box is its body while it bites, and nothing while it is safe",
    "[StrikingPlayer]")
{
    std::vector<std::unique_ptr<Npc>> biting = oneNpcThatBites(2);
    std::vector<std::unique_ptr<Npc>> safe = oneNpcThatBites(0);

    REQUIRE(biting.front()->hurting().has_value());
    REQUIRE(biting.front()->hurting()->box.position == biting.front()->body().aabb().position);
    REQUIRE(biting.front()->hurting()->box.size == biting.front()->body().aabb().size);
    REQUIRE(biting.front()->hurting()->damage == 2);
    REQUIRE_FALSE(safe.front()->hurting().has_value());
}

TEST_CASE("A dead creature hurts with nothing", "[StrikingPlayer]")
{
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(2);
    npcs.front()->takeHit(Hit{99, glm::vec2(1.0f, 0.0f), false});

    REQUIRE_FALSE(npcs.front()->alive());
    REQUIRE_FALSE(npcs.front()->hurting().has_value());
}

TEST_CASE("An npc a tile away does not reach", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1, Here + glm::ivec2(2, 0));

    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A dead npc bites nobody", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    npcs.front()->takeHit(lethalHit());

    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A bite pushes away from the npc", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    player.standAt(npcs.front()->feet() + glm::vec2(2.0f, 0.0f));
    glm::vec2 pushed(0.0f);
    player.onHurt.connect([&] { pushed = player.health().lastHit()->direction; });

    strikePlayer(player, npcs);

    REQUIRE(pushed.x > 0.0f);
}

TEST_CASE("A bite lands once per invulnerable window", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);

    strikePlayer(player, npcs);
    strikePlayer(player, npcs);
    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE(
    "A creature bites while its pounce is in the air, and not on the ground",
    "[StrikingPlayer]")
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
    strikePlayer(player, npcs);
    REQUIRE(player.health().points() == 3);

    for (int step = 0; step < 3; ++step)
    {
        npcs.front()->beginFrame();
        npcs.front()->fixedUpdate(0.01f, level, player.feet() + glm::vec2(4.0f, 0.0f));
    }
    REQUIRE(npcs.front()->decided().pounce.active);
    REQUIRE(npcs.front()->hurting().has_value());
    REQUIRE(npcs.front()->hurting()->box.position == npcs.front()->body().aabb().position);
    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 2);
}

TEST_CASE("A creature with a swing strikes the player with it", "[StrikingPlayer]")
{
    Player player(playerDataWithHealth(3, 0.0f), noIntentions());
    player.standAt(feetOf(SpawnTile + glm::ivec2(1, 0)));

    NpcData swinger = setupNpcData();
    swinger.actorData.motionData.swingAbilityData = SwingAbilityData{};
    swinger.actorData.animationData.clips["attack"] = anAttackClip();
    AnimationWhen whileSwinging;
    whileSwinging.swinging = true;
    swinger.actorData.animationData.ladder = AnimatorData{{{"", "attack", whileSwinging}}};
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

    strikePlayer(player, npcs);

    REQUIRE(player.health().points() == 2);
}
