#include <catch2/catch_test_macros.hpp>
#include <map>
#include <string>
#include "actor/actor_data.hpp"
#include "npc/npc_data.hpp"
#include "player/player_data.hpp"
#include "player/player.hpp"
#include "test_helpers/test_player_utils.hpp"

TEST_CASE("An actor draws from the sheet its data names", "[ActorTextures]")
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
#include "rendering/actor_textures.hpp"
#include "rendering/texture_cache.hpp"

TEST_CASE("Every actor's sheet is loaded before anything draws", "[ActorTextures]")
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

TEST_CASE("An actor that names no sheet is refused by name", "[ActorTextures]")
{
    PlayerData playerData = setupPlayerData();
    playerData.actorData.sheet.texture = std::string(assets::PlayerTexture);

    std::map<std::string, NpcData> npcData{{"villager", NpcData{}}};

    TextureCache textures;

    REQUIRE_THROWS_WITH(
        warmActorTextures(textures, playerData, npcData),
        Catch::Matchers::ContainsSubstring("\"villager\""));
}

#endif
