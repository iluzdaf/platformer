#include <memory>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "actor/health.hpp"
#include "actor/hit.hpp"
#include "helpers/actors.hpp"
#include "helpers/levels.hpp"
#include "helpers/npc_fixtures.hpp"
#include "helpers/tiles.hpp"
#include "npc/npc.hpp"
#include "npc/npc_data.hpp"
#include "npc/touching_npcs.hpp"
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

TEST_CASE("Standing in an npc that bites costs its damage", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(2);

    touchNpcs(player, npcs);

    REQUIRE(player.health().points() == 1);
}

TEST_CASE("An npc with no bite is safe to stand in, and hurts nothing", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(0);
    bool hurt = false;
    player.onHurt.connect([&] { hurt = true; });

    touchNpcs(player, npcs);

    REQUIRE(player.health().points() == 3);
    REQUIRE_FALSE(hurt);
    REQUIRE_FALSE(player.health().invulnerable());
}

TEST_CASE("An npc a tile away does not reach", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1, Here + glm::ivec2(2, 0));

    touchNpcs(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A dead npc bites nobody", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    npcs.front()->takeHit(lethalHit());

    touchNpcs(player, npcs);

    REQUIRE(player.health().points() == 3);
}

TEST_CASE("A bite pushes away from the npc", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);
    player.standAt(npcs.front()->feet() + glm::vec2(2.0f, 0.0f));
    glm::vec2 pushed(0.0f);
    player.onHurt.connect([&] { pushed = player.health().lastHit()->direction; });

    touchNpcs(player, npcs);

    REQUIRE(pushed.x > 0.0f);
}

TEST_CASE("A bite lands once per invulnerable window", "[TouchingNpcs]")
{
    Player player(playerDataWithHealth(3, 1.0f), noIntentions());
    player.standAt(feetOf(Here));
    std::vector<std::unique_ptr<Npc>> npcs = oneNpcThatBites(1);

    touchNpcs(player, npcs);
    touchNpcs(player, npcs);
    touchNpcs(player, npcs);

    REQUIRE(player.health().points() == 2);
}
