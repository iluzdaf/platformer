#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <set>
#include <string>
#include <vector>
#include "ui/armed.hpp"

TEST_CASE("Every pick a panel can arm is told apart from the others", "[Armed]")
{
    constexpr PickTile::For Kinds[] = {
        PickTile::For::PlayerStart,
        PickTile::For::NpcSpawn,
        PickTile::For::PatrolFrom,
        PickTile::For::PatrolTo};

    std::vector<std::string> ids;
    for (PickTile::For what : Kinds)
        for (std::size_t npcIndex = 0; npcIndex < 4; ++npcIndex)
            ids.push_back(pickId(PickTile{what, npcIndex}));

    std::set<std::string> distinct(ids.begin(), ids.end());

    REQUIRE(distinct.size() == ids.size());
}

TEST_CASE("The same pick is always told the same way", "[Armed]")
{
    REQUIRE(
        pickId(PickTile{PickTile::For::PatrolTo, 2}) ==
        pickId(PickTile{PickTile::For::PatrolTo, 2}));
    REQUIRE(
        pickId(PickTile{PickTile::For::PatrolTo, 2}) !=
        pickId(PickTile{PickTile::For::PatrolFrom, 2}));
}
