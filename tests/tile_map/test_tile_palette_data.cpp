#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "animations/frame_animation_data.hpp"
#include "helpers/palettes.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_palette_data.hpp"

TEST_CASE("A palette survives being written and read back", "[TilePalette]")
{
    TileData spike;
    spike.deadly = true;
    spike.collider = TileColliderData{glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f)};
    TileData wall;
    wall.solid = wall.grippable = true;
    TileData torch;
    torch.animationData = FrameAnimationData{{3, 4, 5}, 0.2f};
    TilePalettes palettes =
        theOnlyPalette(paletteOf({{0, TileData{}}, {1, spike}, {2, wall}, {7, torch}}));

    std::string written;
    REQUIRE_FALSE(glz::write_json(palettes, written));

    TilePalettes readBack;
    REQUIRE_FALSE(glz::read_json(readBack, written));

    std::string rewritten;
    REQUIRE_FALSE(glz::write_json(readBack, rewritten));

    REQUIRE(rewritten == written);
    const TilePaletteData &back = readBack.at("default");
    REQUIRE(back.tileSet == palettes.at("default").tileSet);
    REQUIRE(back.tiles.size() == 4);
    REQUIRE(back.tiles.at(1).deadly);
    REQUIRE(back.tiles.at(1).collider->size == glm::vec2(16.0f, 4.0f));
    REQUIRE(back.tiles.at(2).grippable);
    REQUIRE(back.tiles.at(7).animationData->frames == std::vector<int>{3, 4, 5});
}
