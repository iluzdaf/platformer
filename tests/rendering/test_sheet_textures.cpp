#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include "actor/actor_data.hpp"
#include "npc/npc_data.hpp"
#include "player/player_data.hpp"
#include "player/player.hpp"
#include "test_helpers/test_player_utils.hpp"

TEST_CASE("An actor draws from the sheet its data names", "[SheetTextures]")
{
    PlayerData playerData = setupPlayerData();
    playerData.actorData.sheet.texture = "textures/somewhere_else.png";

    Player player(playerData, noIntentions());

    REQUIRE(player.getSheet().texture == "textures/somewhere_else.png");
}

#ifndef SKIP_OPENGL_TESTS

#include <catch2/matchers/catch_matchers_string.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/sheet_textures.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "test_helpers/test_tile_map_utils.hpp"
#include "rendering/texture_cache.hpp"
#include "game/game_data.hpp"
#include "pickups/pickup_data.hpp"
#include "npc/npc_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/actor_animation_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "rendering/texture2d.hpp"

TEST_CASE("Every actor's sheet is loaded before anything draws", "[SheetTextures]")
{
    PlayerData playerData = setupPlayerData();
    playerData.actorData.sheet.texture = std::string(assets::PlayerTexture);

    NpcData villager;
    villager.actorData.sheet.texture = std::string(assets::TileSetTexture);
    std::map<std::string, NpcData> npcData{{"villager", villager}};

    TextureCache textures;
    warmActorTextures(textures, playerData, npcData);

    REQUIRE(textures.find(std::string(assets::PlayerTexture)));
    REQUIRE(textures.find(std::string(assets::TileSetTexture)));
}

TEST_CASE("An actor that names no sheet is refused by name", "[SheetTextures]")
{
    PlayerData playerData = setupPlayerData();
    playerData.actorData.sheet.texture = std::string(assets::PlayerTexture);

    std::map<std::string, NpcData> npcData{{"villager", NpcData{}}};

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmActorTextures(textures, playerData, npcData),
        Catch::Matchers::ContainsSubstring("\"villager\""));
}

TEST_CASE("Two palettes naming two tile sets get two textures", "[SheetTextures]")
{
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});
    palettes["other"] = paletteOf({{0, TileData{}}});
    palettes["other"].tileSet.texture = std::string(assets::PlayerTexture);

    TextureCache textures;
    warmTileSets(textures, palettes);

    const Texture2D &first = textures.get(palettes["default"].tileSet.texture);
    const Texture2D &second = textures.get(palettes["other"].tileSet.texture);

    REQUIRE(&first != &second);
}

TEST_CASE("Two palettes sharing a tile set load it once", "[SheetTextures]")
{
    TilePalettes palettes;
    palettes["default"] = paletteOf({{0, TileData{}}});
    palettes["same"] = paletteOf({{0, TileData{}}});

    TextureCache textures;
    warmTileSets(textures, palettes);

    REQUIRE(
        &textures.get(palettes["default"].tileSet.texture) ==
        &textures.get(palettes["same"].tileSet.texture));
}

TEST_CASE("A palette whose tile set is not on disk says so", "[SheetTextures]")
{
    TilePalettes palettes;
    palettes["ice"] = paletteOf({{0, TileData{}}});
    palettes["ice"].tileSet.texture = "textures/nothing_here.png";

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmTileSets(textures, palettes),
        Catch::Matchers::ContainsSubstring("textures/nothing_here.png"));
}

TEST_CASE("A palette naming no tile set is refused by name", "[SheetTextures]")
{
    TilePalettes palettes;
    palettes["ice"] = paletteOf({{0, TileData{}}});
    palettes["ice"].tileSet.texture.clear();

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmTileSets(textures, palettes),
        Catch::Matchers::ContainsSubstring("No sheet is named") &&
            Catch::Matchers::ContainsSubstring("ice"));
}

TEST_CASE("Every shipped palette fits the tile set it names", "[SheetTextures]")
{
    TextureCache textures;

    REQUIRE_NOTHROW(warmTileSets(textures, loadGameData().tilePalettes));
}

TEST_CASE("Every shipped pickup animates on frames its sheet holds", "[SheetTextures]")
{
    TextureCache textures;

    REQUIRE_NOTHROW(warmPickupTextures(textures, loadGameData().pickupData));
}

TEST_CASE("A pickup animating past the end of its sheet says so", "[SheetTextures]")
{
    std::map<std::string, PickupData> pickupData = loadGameData().pickupData;
    pickupData.at("coin").animationData.frames.push_back(99);

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmPickupTextures(textures, pickupData),
        Catch::Matchers::ContainsSubstring("coin") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

TEST_CASE("Every shipped actor animates on frames its sheet holds", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    TextureCache textures;

    REQUIRE_NOTHROW(warmActorTextures(textures, gameData.playerData, gameData.npcData));
}

TEST_CASE("An actor animating past the end of its sheet says so", "[SheetTextures]")
{
    GameData gameData = loadGameData();
    gameData.npcData.at("villager").actorData.animationData.idle.frames.push_back(99);

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmActorTextures(textures, gameData.playerData, gameData.npcData),
        Catch::Matchers::ContainsSubstring("villager") &&
            Catch::Matchers::ContainsSubstring("frame 99"));
}

#endif
