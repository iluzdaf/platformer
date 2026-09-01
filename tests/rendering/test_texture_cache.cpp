#ifndef SKIP_OPENGL_TESTS

#include <string>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "assets/asset_paths.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"

namespace
{
    const std::string TileSet{assets::TileSetTexture};
    const std::string Player{assets::PlayerTexture};
}

TEST_CASE("A texture warmed twice is loaded once", "[TextureCache]")
{
    TextureCache textures;

    textures.warm(TileSet);
    const Texture2D &first = textures.get(TileSet);
    textures.warm(TileSet);

    REQUIRE(&textures.get(TileSet) == &first);
}

TEST_CASE("Two paths are two textures", "[TextureCache]")
{
    TextureCache textures;

    textures.warm(TileSet);
    textures.warm(Player);

    REQUIRE(&textures.get(TileSet) != &textures.get(Player));
    REQUIRE(textures.get(TileSet).getWidth() == 112);
    REQUIRE(textures.get(Player).getWidth() == 96);
}

TEST_CASE("A path nobody warmed is not found", "[TextureCache]")
{
    TextureCache textures;

    REQUIRE(textures.find(TileSet) == nullptr);
    REQUIRE_THROWS_WITH(textures.get(TileSet), Catch::Matchers::ContainsSubstring(TileSet));
}

TEST_CASE("A texture that is not there says which one", "[TextureCache]")
{
    TextureCache textures;

    REQUIRE_THROWS_WITH(
        textures.warm("textures/nothing_here.png"),
        Catch::Matchers::ContainsSubstring("textures/nothing_here.png"));
}

TEST_CASE("Reloading a path nobody warmed does nothing", "[TextureCache]")
{
    TextureCache textures;

    REQUIRE_NOTHROW(textures.reload("textures/nothing_here.png"));
    REQUIRE(textures.find("textures/nothing_here.png") == nullptr);
}

TEST_CASE("Reloading replaces what was held", "[TextureCache]")
{
    TextureCache textures;
    textures.warm(TileSet);

    textures.reload(TileSet);

    REQUIRE(textures.find(TileSet) != nullptr);
    REQUIRE(textures.get(TileSet).getWidth() == 112);
}

#endif // SKIP_OPENGL_TESTS
