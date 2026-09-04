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
#include "ui/sheet_in_scope.hpp"
#include "assets/sheet_data.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture_cache.hpp"
#include "game/level.hpp"
#include "game/level_data_file.hpp"
#include "test_helpers/asset_path.hpp"

namespace
{
    GameData twoOfEach()
    {
        GameData gameData;
        gameData.npcData = {{"villager", NpcData{}}, {"explorer", NpcData{}}};
        gameData.pickupData = {{"coin", PickupData{}}, {"gem", PickupData{}}};
        return gameData;
    }

    struct TypeRenaming
    {
        TextureCache textures;
        EditorCommands commands;

        auto drawing(TypesUi &typesUi, GameData &gameData)
        {
            return [&] { typesUi.draw(gameData, textures, commands); };
        }
    };
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

    TextureCache textures;
    EditorCommands commands;

    REQUIRE_NOTHROW(gui.frame([&] { typesUi.draw(gameData, textures, commands); }));
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

TEST_CASE("A type hands back the sheet it draws from", "[TypesUi]")
{
    GameData gameData = twoOfEach();
    gameData.pickupData["coin"].sheet.texture = "textures/coin.png";
    gameData.npcData["villager"].actorData.sheet.texture = "textures/player.png";

    REQUIRE(
        sheetOf(gameData, TypeShown{TypeShown::What::Pickup, "coin"})->texture ==
        "textures/coin.png");
    REQUIRE(
        sheetOf(gameData, TypeShown{TypeShown::What::Npc, "villager"})->texture ==
        "textures/player.png");
}

TEST_CASE("A type nobody has hands back no sheet", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    REQUIRE(sheetOf(gameData, TypeShown{TypeShown::What::Npc, "nobody"}) == nullptr);
    REQUIRE(sheetOf(gameData, TypeShown{}) == nullptr);
}

TEST_CASE("Editing a type asks for the sheet it draws from", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;

    std::string asked;
    commands.onWarmTexture.connect([&](const std::string &texture) { asked = texture; });
    gameData.pickupData["coin"].sheet.texture = "textures/coin.png";
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    gui.frame([&] { typesUi.draw(gameData, textures, commands); });
    commands.drain();

    REQUIRE(asked == "textures/coin.png");
}

TEST_CASE("A sheet is only in scope while a type is being drawn", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;

    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    REQUIRE(sheetInScope() == nullptr);
    gui.frame([&] { typesUi.draw(gameData, textures, commands); });
    REQUIRE(sheetInScope() == nullptr);
}

TEST_CASE("A name typed and entered leaves the types unsaved", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "villager"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("A type keeps the name the levels know until it is saved", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "villager"});

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    REQUIRE(gameData.npcData.contains("villager"));
    REQUIRE_FALSE(gameData.npcData.contains("farmer"));
}

TEST_CASE("A type cannot take the name of another of its kind", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "villager"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "explorer", drawing);
    gui.pressEnter(drawing);

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("An npc may take a name a pickup has", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "villager"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "coin", drawing);
    gui.pressEnter(drawing);

    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Reverting takes back a type rename that was never saved", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "penny", drawing);
    gui.pressEnter(drawing);
    REQUIRE(typesUi.unsavedSince(gameData));

    typesUi.revert(gameData);

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    REQUIRE(gameData.pickupData.contains("coin"));
}

TEST_CASE("A level still loads while a type rename waits to be saved", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = loadGameData();
    typesUi.show(TypeShown{TypeShown::What::Npc, "villager"});

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    REQUIRE(typesUi.unsavedSince(gameData));
    REQUIRE_NOTHROW(Level(
        readLevelData(assetPath("levels/level6.json")),
        gameData.tilePalettes,
        gameData.playerData,
        gameData.npcData,
        gameData.pickupData));
}

TEST_CASE("A type rename outlives a reload of the values it waits on", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "penny", drawing);
    gui.pressEnter(drawing);
    REQUIRE(typesUi.unsavedSince(gameData));

    typesUi.reloaded(gameData, gameData);

    REQUIRE(typesUi.unsavedSince(gameData));
}
