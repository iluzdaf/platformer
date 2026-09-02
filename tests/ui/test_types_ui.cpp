#include <catch2/catch_test_macros.hpp>
#include <map>
#include <optional>
#include <string>
#include <imgui_internal.h>
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/type_shown.hpp"
#include "ui/types_ui.hpp"

namespace
{
    GameData twoOfEach()
    {
        GameData gameData;
        gameData.npcData = {{"villager", NpcData{}}, {"explorer", NpcData{}}};
        gameData.pickupData = {{"coin", PickupData{}}, {"gem", PickupData{}}};
        return gameData;
    }
}

TEST_CASE("A name nobody has taken is not one that is", "[TypesUi]")
{
    std::map<std::string, NpcData> types{{"new", NpcData{}}, {"new 2", NpcData{}}};

    REQUIRE(aTypeNameNobodyHasTaken(types) == "new 3");
}

TEST_CASE("A name nobody has taken in an empty catalogue is the first one", "[TypesUi]")
{
    REQUIRE(aTypeNameNobodyHasTaken(std::map<std::string, PickupData>{}) == "new");
}

TEST_CASE("The types section draws with nothing picked", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_NOTHROW(gui.frame([&] { typesUi.draw(gameData); }));
}

TEST_CASE("Nothing is unsaved before a type is touched", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("An npc that changes leaves the section unsaved", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    gameData.npcData["villager"].actorData.size.x += 1.0f;

    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("A pickup that changes leaves the section unsaved", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    gameData.pickupData["coin"].scoreDelta = 99;

    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Reverting puts both kinds back", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    gameData.npcData["villager"].actorData.size.x += 1.0f;
    gameData.pickupData["coin"].scoreDelta = 99;

    typesUi.revert(gameData);

    REQUIRE(gameData.npcData["villager"].actorData.size.x == NpcData{}.actorData.size.x);
    REQUIRE(gameData.pickupData["coin"].scoreDelta == 0);
}

TEST_CASE("Adding an npc makes one nobody had and shows it", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    TypeShown added = addTypeTo(gameData, TypeShown::What::Npc);

    REQUIRE(added.what == TypeShown::What::Npc);
    REQUIRE(gameData.npcData.contains(added.name));
    REQUIRE(gameData.npcData.size() == 3);
    REQUIRE(gameData.pickupData.size() == 2);
}

TEST_CASE("Adding a pickup makes one nobody had and shows it", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    TypeShown added = addTypeTo(gameData, TypeShown::What::Pickup);

    REQUIRE(added.what == TypeShown::What::Pickup);
    REQUIRE(gameData.pickupData.contains(added.name));
    REQUIRE(gameData.pickupData.size() == 3);
    REQUIRE(gameData.npcData.size() == 2);
}

TEST_CASE("Adding twice does not take the same name twice", "[TypesUi]")
{
    GameData gameData;

    TypeShown first = addTypeTo(gameData, TypeShown::What::Pickup);
    TypeShown second = addTypeTo(gameData, TypeShown::What::Pickup);

    REQUIRE(first.name != second.name);
    REQUIRE(gameData.pickupData.size() == 2);
}

TEST_CASE("An npc and a pickup may share a name without sharing a type", "[TypesUi]")
{
    REQUIRE(TypeShown{TypeShown::What::Npc, "coin"} != TypeShown{TypeShown::What::Pickup, "coin"});
}

TEST_CASE("Removing a type takes it and leaves the rest", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    removeTypeFrom(gameData, TypeShown{TypeShown::What::Pickup, "coin"});

    REQUIRE_FALSE(gameData.pickupData.contains("coin"));
    REQUIRE(gameData.pickupData.contains("gem"));
    REQUIRE(gameData.npcData.size() == 2);
}

TEST_CASE("Removing takes the kind it was asked for", "[TypesUi]")
{
    GameData gameData;
    gameData.npcData = {{"coin", NpcData{}}};
    gameData.pickupData = {{"coin", PickupData{}}};

    removeTypeFrom(gameData, TypeShown{TypeShown::What::Npc, "coin"});

    REQUIRE(gameData.npcData.empty());
    REQUIRE(gameData.pickupData.contains("coin"));
}

TEST_CASE("Removing a type nobody has changes nothing", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    removeTypeFrom(gameData, TypeShown{TypeShown::What::Pickup, "nothing"});

    REQUIRE(gameData.pickupData.size() == 2);
}

TEST_CASE("Types that all name a sheet stop no save", "[TypesUi]")
{
    GameData gameData = loadGameData();

    REQUIRE_FALSE(typesNamingNoSheet(gameData));
}

TEST_CASE("A type naming no sheet is named as the reason a save cannot happen", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.pickupData.insert({"unfinished", PickupData{}});

    std::optional<std::string> why = typesNamingNoSheet(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("unfinished"));
}

TEST_CASE("An npc naming no sheet is caught the same way", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.npcData.insert({"faceless", NpcData{}});

    std::optional<std::string> why = typesNamingNoSheet(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("faceless"));
}

TEST_CASE("A type just added names no sheet, so it cannot be saved yet", "[TypesUi]")
{
    GameData gameData = loadGameData();

    TypeShown added = addTypeTo(gameData, TypeShown::What::Pickup);

    REQUIRE(typesNamingNoSheet(gameData)->contains(added.name));
}

TEST_CASE("A type that names a sheet has nothing said against it", "[TypesUi]")
{
    GameData gameData = loadGameData();

    REQUIRE_FALSE(whyATypeCannotBeSaved(gameData, TypeShown{TypeShown::What::Pickup, "coin"}));
}

TEST_CASE("A type that names no sheet says so on its own", "[TypesUi]")
{
    GameData gameData;
    gameData.pickupData = {{"unfinished", PickupData{}}};

    REQUIRE(whyATypeCannotBeSaved(gameData, TypeShown{TypeShown::What::Pickup, "unfinished"}));
}

TEST_CASE("A type nobody has has nothing said against it", "[TypesUi]")
{
    GameData gameData;

    REQUIRE_FALSE(whyATypeCannotBeSaved(gameData, TypeShown{TypeShown::What::Npc, "nobody"}));
}

TEST_CASE("Reverting takes back a type that was added", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    addTypeTo(gameData, TypeShown::What::Pickup);
    addTypeTo(gameData, TypeShown::What::Npc);

    typesUi.revert(gameData);

    REQUIRE(gameData.pickupData.size() == 2);
    REQUIRE(gameData.npcData.size() == 2);
}

TEST_CASE("Reverting puts back a type that was removed", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    removeTypeFrom(gameData, TypeShown{TypeShown::What::Pickup, "coin"});

    typesUi.revert(gameData);

    REQUIRE(gameData.pickupData.contains("coin"));
}
