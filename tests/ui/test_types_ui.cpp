#include <catch2/catch_test_macros.hpp>
#include <vector>
#include <algorithm>
#include <catch2/matchers/catch_matchers_string.hpp>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <fstream>
#include <map>
#include <optional>
#include <string>
#include <imgui_internal.h>
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "npc/npc_spawn_data.hpp"
#include "actor/actor_data.hpp"
#include "assets/asset_paths.hpp"
#include "pickups/pickup_data.hpp"
#include "player/player_data.hpp"
#include "helpers/headless_imgui.hpp"
#include "ui/type_shown.hpp"
#include "ui/types_ui.hpp"
#include "physics/physics_body.hpp"
#include "physics/physics_body_data.hpp"
#include "ui/sheet_in_scope.hpp"
#include "assets/sheet_data.hpp"
#include "ui/editor_commands.hpp"
#include "rendering/texture_cache.hpp"
#include "game/level.hpp"
#include "game/level_data.hpp"
#include "game/level_data_file.hpp"
#include "helpers/asset_path.hpp"
#include "helpers/temporary_levels.hpp"
#include "helpers/levels.hpp"

namespace
{
    std::optional<Armed> armedForTypes;

    GameData twoOfEach()
    {
        GameData gameData;
        gameData.npcData = {{"rat", NpcData{}}, {"spider", NpcData{}}};
        gameData.pickupData = {{"coin", PickupData{}}, {"gem", PickupData{}}};
        return gameData;
    }

    std::unique_ptr<TemporaryLevels> levelsPlacingTypes()
    {
        auto levels = std::make_unique<TemporaryLevels>("type_levels");
        for (const char *name : {"level5.json", "level6.json"})
            levels->copyShipped(name);

        return levels;
    }

    std::string firstNpcTypeIn(const std::filesystem::path &directory)
    {
        return readLevelData((directory / "level6.json").string()).npcs.front().type;
    }

    std::string firstPickupTypeIn(const std::filesystem::path &directory)
    {
        return readLevelData((directory / "level5.json").string()).pickups.front().type;
    }

    struct TypeRenaming
    {
        TextureCache textures;
        EditorCommands commands;

        auto drawing(TypesUi &typesUi, GameData &gameData)
        {
            return [&] { typesUi.draw(gameData, textures, commands, armedForTypes); };
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

    REQUIRE_NOTHROW(gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes); }));
}

TEST_CASE(
    "The shown npc's machine draws, lit by the creatures of its type in the level",
    "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = loadGameData();
    Level level = levelWithALedgeAndAWall({spawnAt("rat", ledge_and_wall::LedgeRightEnd)});
    TextureCache textures;
    EditorCommands commands;
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

    REQUIRE_NOTHROW(
        gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes, &level); }));
    REQUIRE_NOTHROW(gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes); }));
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
    gameData.npcData["rat"].actorData.size = glm::vec2(24.0f);

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
    gameData.npcData["rat"].actorData.size = glm::vec2(24.0f);
    gameData.pickupData["coin"].scoreDelta = 99;

    typesUi.revert(gameData);

    REQUIRE(gameData.npcData["rat"].actorData.size == NpcData{}.actorData.size);
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

    REQUIRE_FALSE(aTypeThatCannotBeSaved(gameData));
}

TEST_CASE("A type naming no sheet is named as the reason a save cannot happen", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.pickupData.insert({"unfinished", PickupData{}});

    std::optional<std::string> why = aTypeThatCannotBeSaved(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("unfinished"));
}

TEST_CASE("An npc naming no sheet is caught the same way", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.npcData.insert({"faceless", NpcData{}});

    std::optional<std::string> why = aTypeThatCannotBeSaved(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("faceless"));
}

TEST_CASE("An npc whose body nothing can build is the reason a save cannot happen", "[TypesUi]")
{
    GameData gameData = loadGameData();
    PhysicsBodyData &body = gameData.npcData.at("rat").actorData.physicsBodyData;
    body.colliderSize = glm::vec2(5.0f, 4.0f);
    body.stepHeight = 3.0f;

    std::optional<std::string> why = aTypeThatCannotBeSaved(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("rat"));
    REQUIRE(why->contains("leaves nothing of the body"));
}

TEST_CASE("The player whose body nothing can build stops the save too", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.playerData.actorData.physicsBodyData.colliderSize = glm::vec2(0.0f, 13.0f);

    std::optional<std::string> why = aTypeThatCannotBeSaved(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("player"));
    REQUIRE(why->contains("not one anything can touch"));
}

TEST_CASE("A body the editor refuses is one the game would refuse too", "[TypesUi]")
{
    PhysicsBodyData body;
    body.colliderSize = glm::vec2(5.0f, 4.0f);
    body.stepHeight = 3.0f;

    REQUIRE(whyNotABody(body));
    REQUIRE_THROWS(PhysicsBody(body));

    body.stepHeight = 1.0f;

    REQUIRE_FALSE(whyNotABody(body));
    REQUIRE_NOTHROW(PhysicsBody(body));
}

TEST_CASE("A pickup has no body to refuse, not even somebody else's", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.playerData.actorData.physicsBodyData.colliderSize = glm::vec2(0.0f);

    REQUIRE(whyATypeCannotBeSaved(gameData, thePlayer()));
    REQUIRE_FALSE(whyATypeCannotBeSaved(gameData, TypeShown{TypeShown::What::Pickup, "coin"}));
}

TEST_CASE("A type just added names no sheet, so it cannot be saved yet", "[TypesUi]")
{
    GameData gameData = loadGameData();

    TypeShown added = addTypeTo(gameData, TypeShown::What::Pickup);

    REQUIRE(aTypeThatCannotBeSaved(gameData)->contains(added.name));
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

TEST_CASE("The player is in the cast and hands back their own sheet", "[TypesUi]")
{
    GameData gameData = twoOfEach();
    gameData.playerData.actorData.sheet.texture.path = "textures/hero.png";

    REQUIRE(sheetOf(gameData, thePlayer()) == &gameData.playerData.actorData.sheet);
    REQUIRE(sheetOf(gameData, thePlayer())->texture == "textures/hero.png");
}

TEST_CASE("The player cannot leave the cast", "[TypesUi]")
{
    GameData gameData = twoOfEach();

    REQUIRE_THROWS(removeTypeFrom(gameData, thePlayer()));
    REQUIRE_THROWS(addTypeTo(gameData, TypeShown::What::Player));
}

TEST_CASE("The player naming no sheet is named as the reason a save cannot happen", "[TypesUi]")
{
    GameData gameData = loadGameData();
    gameData.playerData.actorData.sheet.texture.path.clear();

    std::optional<std::string> why = aTypeThatCannotBeSaved(gameData);

    REQUIRE(why);
    REQUIRE(why->contains("player"));
}

TEST_CASE("The player that changes leaves the cast unsaved", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    gameData.playerData.fallFromHeightThreshold += 1.0f;

    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Reverting puts the player back", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    float was = gameData.playerData.fallFromHeightThreshold;

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    gameData.playerData.fallFromHeightThreshold += 1.0f;

    typesUi.revert(gameData);

    REQUIRE(gameData.playerData.fallFromHeightThreshold == was);
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Saving the cast writes the player only when they changed", "[TypesUi]")
{
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    std::optional<PlayerData> written;
    TypesUi typesUi(
        levels->directory.string(),
        [](const std::map<std::string, NpcData> &) {},
        [](const std::map<std::string, PickupData> &) {},
        [&](const PlayerData &player) { written = player; });
    GameData gameData = twoOfEach();
    LevelData playing = readLevelData((levels->directory / "level6.json").string());
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.save(gameData, playing);
    REQUIRE_FALSE(written.has_value());

    gameData.playerData.fallFromHeightThreshold += 1.0f;
    typesUi.save(gameData, playing);

    REQUIRE(written.has_value());
    REQUIRE(written->fallFromHeightThreshold == gameData.playerData.fallFromHeightThreshold);
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("A reload keeps an unsaved player edit and follows the disk otherwise", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    GameData onDisk = gameData;
    onDisk.playerData.fallFromHeightThreshold += 50.0f;

    typesUi.reloaded(gameData, onDisk);
    REQUIRE(
        gameData.playerData.fallFromHeightThreshold == onDisk.playerData.fallFromHeightThreshold);
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    gameData.playerData.fallFromHeightThreshold += 1.0f;
    float edited = gameData.playerData.fallFromHeightThreshold;
    onDisk.playerData.fallFromHeightThreshold += 50.0f;

    typesUi.reloaded(gameData, onDisk);

    REQUIRE(gameData.playerData.fallFromHeightThreshold == edited);
    REQUIRE(typesUi.unsavedSince(gameData));
}

TEST_CASE("The cast starts on the player", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    gameData.playerData.actorData.sheet.texture.path = "textures/hero.png";

    std::string asked;
    commands.onWarmTexture.connect([&](const std::string &texture) { asked = texture; });
    gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes); });
    commands.drain();

    REQUIRE(asked == "textures/hero.png");
}

TEST_CASE("A type hands back the sheet it draws from", "[TypesUi]")
{
    GameData gameData = twoOfEach();
    gameData.pickupData["coin"].sheet.texture.path = "textures/coin.png";
    gameData.npcData["rat"].actorData.sheet.texture.path = "textures/player.png";

    REQUIRE(
        sheetOf(gameData, TypeShown{TypeShown::What::Pickup, "coin"})->texture ==
        "textures/coin.png");
    REQUIRE(
        sheetOf(gameData, TypeShown{TypeShown::What::Npc, "rat"})->texture ==
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
    gameData.pickupData["coin"].sheet.texture.path = "textures/coin.png";
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes); });
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
    gui.frame([&] { typesUi.draw(gameData, textures, commands, armedForTypes); });
    REQUIRE(sheetInScope() == nullptr);
}

TEST_CASE("A name typed and entered leaves the types unsaved", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

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
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    REQUIRE(gameData.npcData.contains("rat"));
    REQUIRE_FALSE(gameData.npcData.contains("farmer"));
}

TEST_CASE("A type cannot take the name of another of its kind", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.type("##name", "spider", drawing);
    gui.pressEnter(drawing);

    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("An npc may take a name a pickup has", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

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
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

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

TEST_CASE("Saving a type rename re-points the levels before the types are written", "[TypesUi]")
{
    HeadlessImGui gui;
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    std::optional<std::map<std::string, NpcData>> written;
    TypesUi typesUi(
        directory.string(),
        [&](const std::map<std::string, NpcData> &npcs)
        {
            REQUIRE(firstNpcTypeIn(directory) == "farmer");
            written = npcs;
        },
        [](const std::map<std::string, PickupData> &) {});
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);
    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    LevelData playing = readLevelData((directory / "level6.json").string());
    REQUIRE(typesUi.save(gameData, playing));

    REQUIRE(written.has_value());
    REQUIRE(written->contains("farmer"));
    REQUIRE_FALSE(written->contains("rat"));
    REQUIRE(playing.npcs.front().type == "farmer");
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Saving a pickup rename re-points the level being played", "[TypesUi]")
{
    HeadlessImGui gui;
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    bool wrote = false;
    TypesUi typesUi(
        directory.string(),
        [](const std::map<std::string, NpcData> &) {},
        [&](const std::map<std::string, PickupData> &)
        {
            REQUIRE(firstPickupTypeIn(directory) == "penny");
            wrote = true;
        });
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);
    gui.type("##name", "penny", drawing);
    gui.pressEnter(drawing);

    LevelData playing = readLevelData((directory / "level5.json").string());
    REQUIRE(typesUi.save(gameData, playing));

    REQUIRE(wrote);
    REQUIRE(playing.pickups.front().type == "penny");
}

TEST_CASE("A type save with nothing pending leaves the playing level alone", "[TypesUi]")
{
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    bool wrote = false;
    TypesUi typesUi(
        directory.string(),
        [&](const std::map<std::string, NpcData> &) { wrote = true; },
        [&](const std::map<std::string, PickupData> &) { wrote = true; });
    GameData gameData = twoOfEach();
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    LevelData playing = readLevelData((directory / "level6.json").string());
    REQUIRE_FALSE(typesUi.save(gameData, playing));

    REQUIRE_FALSE(wrote);
    REQUIRE(playing.npcs.front().type == firstNpcTypeIn(directory));
}

TEST_CASE(
    "A type removed stays until the save, is not offered, and leaves the cast unsaved",
    "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    typesUi.remove(gameData);

    REQUIRE(gameData.npcData.contains("rat"));
    REQUIRE(typesUi.unsavedSince(gameData));
    REQUIRE(gameData.npcData.size() == 2);
}

TEST_CASE(
    "Saving a type removal drops its placements from every level, and the one being played",
    "[TypesUi]")
{
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    std::optional<std::map<std::string, NpcData>> written;
    TypesUi typesUi(
        directory.string(),
        [&](const std::map<std::string, NpcData> &npcs)
        {
            REQUIRE(
                readLevelData((directory / "level6.json").string()).npcs.size() <
                readLevelData(assetPath("levels/level6.json")).npcs.size());
            written = npcs;
        },
        [](const std::map<std::string, PickupData> &) {});
    GameData gameData = twoOfEach();
    std::string type = firstNpcTypeIn(directory);
    std::size_t placedBefore = readLevelData((directory / "level6.json").string()).npcs.size();
    typesUi.show(TypeShown{TypeShown::What::Npc, type});
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.remove(gameData);

    LevelData playing = readLevelData((directory / "level6.json").string());
    REQUIRE(typesUi.save(gameData, playing));

    REQUIRE(written.has_value());
    REQUIRE_FALSE(written->contains(type));
    REQUIRE_FALSE(written->contains(""));
    REQUIRE_FALSE(gameData.npcData.contains(type));
    REQUIRE_FALSE(gameData.npcData.contains(""));
    for (const NpcSpawnData &spawn : readLevelData((directory / "level6.json").string()).npcs)
        REQUIRE(spawn.type != type);
    for (const NpcSpawnData &spawn : playing.npcs)
        REQUIRE(spawn.type != type);
    REQUIRE(playing.npcs.size() < placedBefore);
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("A type added and removed before a save vanishes at once", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.add(gameData, TypeShown::What::Pickup);
    REQUIRE(gameData.pickupData.size() == 3);

    typesUi.remove(gameData);

    REQUIRE(gameData.pickupData.size() == 2);
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("Reverting takes back a type removal that was never saved", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});
    typesUi.remove(gameData);
    typesUi.revert(gameData);

    REQUIRE(gameData.pickupData.contains("coin"));
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
}

TEST_CASE("A type removal cannot be saved while a level cannot be read", "[TypesUi]")
{
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    std::ofstream(directory / "broken.json") << "{";
    bool wrote = false;
    TypesUi typesUi(
        directory.string(),
        [&](const std::map<std::string, NpcData> &) { wrote = true; },
        [&](const std::map<std::string, PickupData> &) { wrote = true; });
    GameData gameData = twoOfEach();
    for (auto &[name, npc] : gameData.npcData)
        npc.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    for (auto &[name, pickup] : gameData.pickupData)
        pickup.sheet.texture.path = std::string(assets::PlayerTexture);
    gameData.playerData.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    std::string type = firstNpcTypeIn(directory);
    typesUi.show(TypeShown{TypeShown::What::Npc, type});
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    typesUi.remove(gameData);

    REQUIRE(typesUi.cannotSaveBecause(gameData) == "broken cannot be read");
    LevelData playing = readLevelData((directory / "level6.json").string());
    REQUIRE_FALSE(typesUi.save(gameData, playing));
    REQUIRE_FALSE(wrote);
    REQUIRE(gameData.npcData.contains(type));
    REQUIRE(firstNpcTypeIn(directory) == type);
}

TEST_CASE("The chooser offers the player, then the npcs, then the pickups", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    std::vector<TypeShown> offered = typesUi.offered(gameData);

    REQUIRE(
        offered == std::vector<TypeShown>{
                       thePlayer(),
                       TypeShown{TypeShown::What::Npc, "rat"},
                       TypeShown{TypeShown::What::Npc, "spider"},
                       TypeShown{TypeShown::What::Pickup, "coin"},
                       TypeShown{TypeShown::What::Pickup, "gem"}});
}

TEST_CASE("A type removed is no longer offered, and is offered again once reverted", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});

    typesUi.remove(gameData);

    std::vector<TypeShown> offered = typesUi.offered(gameData);
    REQUIRE(
        std::find(offered.begin(), offered.end(), TypeShown{TypeShown::What::Npc, "rat"}) ==
        offered.end());
    REQUIRE(
        std::find(offered.begin(), offered.end(), TypeShown{TypeShown::What::Npc, "spider"}) !=
        offered.end());
    REQUIRE(gameData.npcData.contains("rat"));

    typesUi.revert(gameData);

    offered = typesUi.offered(gameData);
    REQUIRE(
        std::find(offered.begin(), offered.end(), TypeShown{TypeShown::What::Npc, "rat"}) !=
        offered.end());
}

TEST_CASE("A type rename cannot be saved while a level cannot be read", "[TypesUi]")
{
    HeadlessImGui gui;
    std::unique_ptr<TemporaryLevels> levels = levelsPlacingTypes();
    const std::filesystem::path &directory = levels->directory;
    std::ofstream(directory / "broken.json") << "{";
    bool wrote = false;
    TypesUi typesUi(
        directory.string(),
        [&](const std::map<std::string, NpcData> &) { wrote = true; },
        [&](const std::map<std::string, PickupData> &) { wrote = true; });
    GameData gameData = twoOfEach();
    for (auto &[name, npc] : gameData.npcData)
        npc.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    for (auto &[name, pickup] : gameData.pickupData)
        pickup.sheet.texture.path = std::string(assets::PlayerTexture);
    gameData.playerData.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    REQUIRE_FALSE(typesUi.unsavedSince(gameData));

    TypeRenaming renaming;
    auto drawing = renaming.drawing(typesUi, gameData);
    gui.type("##name", "farmer", drawing);
    gui.pressEnter(drawing);

    REQUIRE(typesUi.cannotSaveBecause(gameData) == "broken cannot be read");

    LevelData playing = readLevelData((directory / "level6.json").string());
    REQUIRE_FALSE(typesUi.save(gameData, playing));

    REQUIRE_FALSE(wrote);
    REQUIRE(gameData.npcData.contains("rat"));
    REQUIRE(playing.npcs.front().type == "rat");
    REQUIRE(firstNpcTypeIn(directory) == "rat");
    REQUIRE(typesUi.unsavedSince(gameData));
}

#ifndef SKIP_OPENGL_TESTS

#include "helpers/pictures_drawn.hpp"
#include "ui/sheet_preview.hpp"
#include "ui/armed.hpp"

TEST_CASE("An actor's preview shows the collider its body will have", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    gameData.playerData.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    gameData.playerData.actorData.sheet.cellSize = glm::ivec2(32);
    gameData.playerData.actorData.size = glm::vec2(16.0f);
    gameData.playerData.actorData.physicsBodyData.colliderSize = glm::vec2(8.0f, 13.0f);
    textures.warm(std::string(assets::PlayerTexture));
    typesUi.show(thePlayer());
    auto drawing = [&] { typesUi.draw(gameData, textures, commands, armedForTypes); };

    REQUIRE(drawsAPictureWide(gui, PreviewSize * 8.0f / 16.0f, drawing));
}

TEST_CASE("A pickup's preview shows the reach that collects it", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    PickupData &coin = gameData.pickupData["coin"];
    coin.sheet.texture.path = std::string(assets::PlayerTexture);
    coin.size = glm::vec2(16.0f);
    coin.colliderSize = glm::vec2(6.0f);
    textures.warm(std::string(assets::PlayerTexture));
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});
    auto drawing = [&] { typesUi.draw(gameData, textures, commands, armedForTypes); };

    REQUIRE(drawsAPictureWide(gui, PreviewSize * 6.0f / 16.0f, drawing));
}

TEST_CASE("The types section previews an npc above its fields", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    gameData.npcData["rat"].actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    auto drawing = [&] { typesUi.draw(gameData, textures, commands, armedForTypes); };

    REQUIRE_FALSE(drawsAPictureWide(gui, PreviewSize, drawing));

    textures.warm(std::string(assets::PlayerTexture));

    REQUIRE(drawsAPictureWide(gui, PreviewSize, drawing));
}

TEST_CASE("The types section previews the player above their fields", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    gameData.playerData.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    textures.warm(std::string(assets::PlayerTexture));
    typesUi.show(thePlayer());

    REQUIRE(drawsAPictureWide(
        gui, PreviewSize, [&] { typesUi.draw(gameData, textures, commands, armedForTypes); }));
}

TEST_CASE("The types section previews a pickup above its fields", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    TextureCache textures;
    EditorCommands commands;
    gameData.pickupData["coin"].sheet.texture.path = std::string(assets::PlayerTexture);
    textures.warm(std::string(assets::PlayerTexture));
    typesUi.show(TypeShown{TypeShown::What::Pickup, "coin"});

    REQUIRE(drawsAPictureWide(
        gui, PreviewSize, [&] { typesUi.draw(gameData, textures, commands, armedForTypes); }));
}

#endif

TEST_CASE("An edit committed in the cast panel asks for the cast to change", "[TypesUi]")
{
    HeadlessImGui gui;
    TypesUi typesUi;
    GameData gameData = twoOfEach();
    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    TypeRenaming renaming;
    int castChanged = 0;
    renaming.commands.onCastChanged.connect([&] { ++castChanged; });
    auto drawing = renaming.drawing(typesUi, gameData);

    gui.frame(drawing);
    renaming.commands.drain();
    REQUIRE(castChanged == 0);

    gui.frame(
        [&]
        {
            ImGui::TreeNodeSetOpen(ImGui::GetID("actorData"), true);
            ImGui::PushOverrideID(ImGui::GetID("actorData"));
            ImGui::TreeNodeSetOpen(ImGui::GetID("motionData"), true);
            ImGui::PushOverrideID(ImGui::GetID("motionData"));
            ImGui::ActivateItemByID(ImGui::GetID("jumpAbilityData"));
            ImGui::PopID();
            ImGui::PopID();
            drawing();
        });
    gui.frame(drawing);
    gui.frame(drawing);
    renaming.commands.drain();

    REQUIRE(gameData.npcData.at("rat").actorData.motionData.jumpAbilityData.has_value());
    REQUIRE(castChanged == 1);
}

TEST_CASE(
    "A type the game could not build cannot be saved, and says why in its own words",
    "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = loadGameData();
    REQUIRE_FALSE(typesUi.cannotSaveBecause(gameData).has_value());

    gameData.pickupData.begin()->second.size = glm::vec2(0.0f);

    std::optional<std::string> why = typesUi.cannotSaveBecause(gameData);
    REQUIRE(why.has_value());
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring(gameData.pickupData.begin()->first));
    REQUIRE_THAT(*why, Catch::Matchers::ContainsSubstring("nobody can see"));
    REQUIRE(whyATypeCannotBeSaved(
                gameData, TypeShown{TypeShown::What::Pickup, gameData.pickupData.begin()->first})
                .has_value());
}

TEST_CASE("A type added or removed says the names changed, once", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    REQUIRE_FALSE(typesUi.namesChanged());

    typesUi.add(gameData, TypeShown::What::Npc);

    REQUIRE(typesUi.namesChanged());
    REQUIRE_FALSE(typesUi.namesChanged());

    typesUi.show(TypeShown{TypeShown::What::Npc, "rat"});
    typesUi.remove(gameData);

    REQUIRE(typesUi.namesChanged());
    REQUIRE_FALSE(typesUi.namesChanged());
}

TEST_CASE("Editing a type without touching its name says nothing changed", "[TypesUi]")
{
    TypesUi typesUi;
    GameData gameData = twoOfEach();

    gameData.npcData.at("rat").actorData.size = glm::vec2(24.0f);

    REQUIRE_FALSE(typesUi.namesChanged());
}
