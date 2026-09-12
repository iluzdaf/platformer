#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
#include "player/player.hpp"
#include "helpers/palettes.hpp"
#include "helpers/actors.hpp"

TEST_CASE("An actor draws from the sheet its data names", "[SheetTextures]")
{
    PlayerData playerData = playerDataWithEveryAbility();
    playerData.actorData.sheet.texture.path = "textures/somewhere_else.png";

    Player player(playerData, noIntentions());

    REQUIRE(player.drawnFrom().texture.path == "textures/somewhere_else.png");
}

#ifndef SKIP_OPENGL_TESTS

#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/sheet_textures.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "rendering/texture_cache.hpp"
#include "game/game_data.hpp"
#include "actor/actor_data.hpp"
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "assets/sheet_data.hpp"

TEST_CASE("An actor's sheet is loaded and checked together", "[SheetTextures]")
{
    ActorData actor;
    actor.sheet.texture.path = std::string(assets::PlayerTexture);

    TextureCache textures;
    warmAndCheck(textures, actor, "the player");

    REQUIRE(textures.find(std::string(assets::PlayerTexture)));
}

TEST_CASE("An actor that names no sheet is refused by name", "[SheetTextures]")
{
    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmAndCheck(textures, ActorData{}, "npcData \"rat\""),
        Catch::Matchers::ContainsSubstring("No sheet is named") &&
            Catch::Matchers::ContainsSubstring("\"rat\""));
}

TEST_CASE("Two palettes naming two tile sets get two textures", "[SheetTextures]")
{
    TilePaletteData first = paletteOf({{0, TileData{}}});
    TilePaletteData second = paletteOf({{0, TileData{}}});
    second.tileSet.texture.path = std::string(assets::PlayerTexture);

    TextureCache textures;
    warmAndCheck(textures, first, "\"default\"");
    warmAndCheck(textures, second, "\"other\"");

    REQUIRE(
        &textures.get(first.tileSet.texture.path) != &textures.get(second.tileSet.texture.path));
}

TEST_CASE("Two palettes sharing a tile set load it once", "[SheetTextures]")
{
    TilePaletteData first = paletteOf({{0, TileData{}}});
    TilePaletteData same = paletteOf({{0, TileData{}}});

    TextureCache textures;
    warmAndCheck(textures, first, "\"default\"");
    warmAndCheck(textures, same, "\"same\"");

    REQUIRE(&textures.get(first.tileSet.texture.path) == &textures.get(same.tileSet.texture.path));
}

TEST_CASE("A palette whose tile set is not on disk says so", "[SheetTextures]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}});
    palette.tileSet.texture.path = "textures/nothing_here.png";

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmAndCheck(textures, palette, "\"ice\""),
        Catch::Matchers::ContainsSubstring("textures/nothing_here.png"));
}

TEST_CASE("A palette naming no tile set is refused by name", "[SheetTextures]")
{
    TilePaletteData palette = paletteOf({{0, TileData{}}});
    palette.tileSet.texture.path.clear();

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmAndCheck(textures, palette, "tilePalettes \"ice\""),
        Catch::Matchers::ContainsSubstring("No sheet is named") &&
            Catch::Matchers::ContainsSubstring("ice"));
}

TEST_CASE("Everything the shipped data names fits the sheet it names", "[SheetTextures]")
{
    TextureCache textures;

    REQUIRE_NOTHROW(checkEverythingFits(textures, loadGameData()));
}

TEST_CASE("A pickup animating past the end of its sheet says so", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    gameData.pickupData.at("coin").animationData.frames.push_back(99);

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        checkEverythingFits(textures, gameData),
        Catch::Matchers::ContainsSubstring("coin") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

TEST_CASE("An actor animating past the end of its sheet says so", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    gameData.npcData.at("rat").actorData.animationData->clips.at("idle").frames.push_back(99);

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        checkEverythingFits(textures, gameData),
        Catch::Matchers::ContainsSubstring("rat") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

TEST_CASE("An animation an actor rarely plays is checked like any other", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    gameData.npcData.at("rat").actorData.animationData->clips["attack"] =
        FrameAnimationData{{99}, 0.1f};

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        checkEverythingFits(textures, gameData),
        Catch::Matchers::ContainsSubstring("rat") && Catch::Matchers::ContainsSubstring("attack") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

TEST_CASE("A health icon naming a heart its sheet has not got says so", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    gameData.settings.healthIcon.spent = 99;

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        checkEverythingFits(textures, gameData),
        Catch::Matchers::ContainsSubstring("health") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

TEST_CASE("Warming the data loads every texture it names", "[SheetTextures]")
{
    GameData gameData;
    gameData.playerData.actorData.sheet.texture.path = std::string(assets::PlayerTexture);
    gameData.settings.healthIcon.sheet.texture.path = std::string(assets::TileSetTexture);
    TextureCache textures;

    warmEverySheetIn(textures, gameData);

    REQUIRE(textures.find(std::string(assets::PlayerTexture)));
    REQUIRE(textures.find(std::string(assets::TileSetTexture)));
}

TEST_CASE("Data naming no sheet at all is warmed without complaint", "[SheetTextures]")
{
    TextureCache textures;

    REQUIRE_NOTHROW(warmEverySheetIn(textures, GameData{}));
}

#endif
