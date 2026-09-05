#include <catch2/catch_test_macros.hpp>
#include <string>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include "actor/actor_animation_data.hpp"
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"
#include "ui/sheet_preview.hpp"

namespace
{
    constexpr float CellSize = 16.0f;
    constexpr float Scale = PreviewSize / CellSize;

    std::vector<std::string> namesOf(const std::vector<NamedAnimation> &offered)
    {
        std::vector<std::string> names;
        for (const NamedAnimation &animation : offered)
            names.emplace_back(animation.name);
        return names;
    }
}

TEST_CASE("A preview shows the frame the time has reached", "[SheetPreview]")
{
    FrameAnimationData animation{{10, 11, 12}, 0.25f};

    REQUIRE(previewFrameAt(animation, 0.0, 7) == 10);
    REQUIRE(previewFrameAt(animation, 0.24, 7) == 10);
    REQUIRE(previewFrameAt(animation, 0.25, 7) == 11);
    REQUIRE(previewFrameAt(animation, 0.5, 7) == 12);
}

TEST_CASE("A preview wraps when the frames run out", "[SheetPreview]")
{
    FrameAnimationData animation{{10, 11, 12}, 0.25f};

    REQUIRE(previewFrameAt(animation, 0.75, 7) == 10);
    REQUIRE(previewFrameAt(animation, 1.0, 7) == 11);
}

TEST_CASE("A preview of nothing stands on the tile being edited", "[SheetPreview]")
{
    FrameAnimationData nothing;

    REQUIRE(previewFrameAt(nothing, 3.0, 42) == 42);
}

TEST_CASE("A preview with no duration stands still rather than dividing by it", "[SheetPreview]")
{
    FrameAnimationData stopped{{10, 11}, 0.0f};

    REQUIRE(previewFrameAt(stopped, 3.0, 42) == 42);
    REQUIRE(previewFrameAt(stopped, 0.0, 42) == 42);
}

TEST_CASE("An animation with no duration does not spin", "[SheetPreview]")
{
    FrameAnimation stopped(FrameAnimationData{{10, 11}, 0.0f});

    stopped.update(1.0f);

    REQUIRE(stopped.getCurrentFrame() == 10);
}

TEST_CASE("A collider covering the whole tile covers the whole preview", "[SheetPreview]")
{
    auto [low, high] = colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f), glm::vec2(16.0f));

    REQUIRE(low.x == 0.0f);
    REQUIRE(low.y == 0.0f);
    REQUIRE(high.x == PreviewSize);
    REQUIRE(high.y == PreviewSize);
}

TEST_CASE("A spike's collider lies along the bottom of its tile", "[SheetPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f, 12.0f), glm::vec2(16.0f, 4.0f));

    REQUIRE(low.y == PreviewSize * 0.75f);
    REQUIRE(high.y == PreviewSize);
    REQUIRE(low.x == 0.0f);
    REQUIRE(high.x == PreviewSize);
}

TEST_CASE("A collider up the left side stays up the left side", "[SheetPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(0.0f, 0.0f), Scale, glm::vec2(0.0f), glm::vec2(4.0f, 16.0f));

    REQUIRE(high.x == PreviewSize * 0.25f);
    REQUIRE(high.y == PreviewSize);
}

TEST_CASE("A collider is drawn from where the tile is", "[SheetPreview]")
{
    auto [low, high] =
        colliderRect(ImVec2(30.0f, 70.0f), Scale, glm::vec2(4.0f, 4.0f), glm::vec2(8.0f));

    REQUIRE(low.x == 30.0f + 24.0f);
    REQUIRE(low.y == 70.0f + 24.0f);
    REQUIRE(high.x == low.x + 48.0f);
    REQUIRE(high.y == low.y + 48.0f);
}

TEST_CASE("An actor offers idle and whichever animations it has", "[SheetPreview]")
{
    ActorAnimationData animations;
    animations.walk = FrameAnimationData{{1, 2}, 0.1f};
    animations.fall = FrameAnimationData{{3}, 0.1f};

    std::vector<NamedAnimation> offered = animationsOf(animations);

    REQUIRE(namesOf(offered) == std::vector<std::string>{"idle", "walk", "fall"});
    REQUIRE(offered[1].animation == &*animations.walk);
    REQUIRE(offered[2].animation == &*animations.fall);
}

TEST_CASE("An animation asked for by name is the one offered under it", "[SheetPreview]")
{
    ActorAnimationData animations;
    animations.walk = FrameAnimationData{{1, 2}, 0.1f};

    std::vector<NamedAnimation> offered = animationsOf(animations);

    REQUIRE(animationNamed(offered, "walk").animation == &*animations.walk);
}

TEST_CASE("An animation nobody has is previewed as idle", "[SheetPreview]")
{
    ActorAnimationData animations;
    animations.walk = FrameAnimationData{{1, 2}, 0.1f};

    std::vector<NamedAnimation> offered = animationsOf(animations);

    REQUIRE(animationNamed(offered, "dash").animation == &animations.idle);
}

TEST_CASE("Nothing offered is nothing to preview", "[SheetPreview]")
{
    REQUIRE_THROWS(animationNamed({}, "idle"));
}

#ifndef SKIP_OPENGL_TESTS

#include <algorithm>
#include <optional>
#include <utility>
#include <imgui_internal.h>
#include "assets/sheet_data.hpp"
#include "rendering/texture2d.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "test_helpers/made_sheet.hpp"
#include "tile_map/tile_data.hpp"
#include "ui/sheet_in_scope.hpp"

namespace
{
    struct Drawn
    {
        int vertices = 0;
        std::vector<std::pair<float, float>> uvs;
        ImVec2 size;
    };

    template <class Draw> Drawn drawnBy(HeadlessImGui &gui, Draw &&draw)
    {
        Drawn drawn;
        gui.frame(draw);
        gui.frame(
            [&]
            {
                ImDrawList *drawList = ImGui::GetWindowDrawList();
                int before = drawList->VtxBuffer.Size;
                draw();
                drawn.size = ImGui::GetItemRectSize();
                drawn.vertices = drawList->VtxBuffer.Size - before;
                for (int at = before; at < drawList->VtxBuffer.Size; ++at)
                    drawn.uvs.emplace_back(
                        drawList->VtxBuffer[at].uv.x, drawList->VtxBuffer[at].uv.y);
            });
        return drawn;
    }
}

TEST_CASE("A tile preview plays the tile's animation", "[SheetPreview]")
{
    HeadlessImGui gui;
    Texture2D sheet = aSheetOf(7, 6);
    SheetInScope offering{&sheet, SheetData{"textures/somewhere.png", glm::ivec2(16)}};

    TileData still;
    TileData animated;
    animated.animationData = FrameAnimationData{{5}, 0.1f};
    FrameAnimationData onFive{{5}, 0.1f};

    Drawn stillDrawn = drawnBy(gui, [&] { drawTilePreview(offering, 0, still); });
    Drawn animatedDrawn = drawnBy(gui, [&] { drawTilePreview(offering, 0, animated); });
    Drawn frameFive = drawnBy(gui, [&] { drawAnimationPreview(offering, onFive); });

    REQUIRE(animatedDrawn.uvs != stillDrawn.uvs);
    REQUIRE(animatedDrawn.uvs.size() > frameFive.uvs.size());
    REQUIRE(std::equal(frameFive.uvs.begin(), frameFive.uvs.end(), animatedDrawn.uvs.begin()));
}

TEST_CASE("A tile preview draws the collider over the picture", "[SheetPreview]")
{
    HeadlessImGui gui;
    Texture2D sheet = aSheetOf(7, 6);
    SheetInScope offering{&sheet, SheetData{"textures/somewhere.png", glm::ivec2(16)}};
    TileData tile;
    FrameAnimationData nothing;

    Drawn asTile = drawnBy(gui, [&] { drawTilePreview(offering, 0, tile); });
    Drawn asFrame = drawnBy(gui, [&] { drawAnimationPreview(offering, nothing); });

    REQUIRE(asTile.vertices > asFrame.vertices);
}

TEST_CASE("A preview keeps the shape of the frame", "[SheetPreview]")
{
    HeadlessImGui gui;
    Texture2D sheet = aSheetOf(7, 6);
    SheetInScope tall{&sheet, SheetData{"textures/somewhere.png", glm::ivec2(16, 24)}};
    FrameAnimationData nothing;

    Drawn drawn = drawnBy(gui, [&] { drawAnimationPreview(tall, nothing); });

    REQUIRE(drawn.size.x == PreviewSize);
    REQUIRE(drawn.size.y == PreviewSize * 1.5f);
}

TEST_CASE("A preview with no texture draws nothing", "[SheetPreview]")
{
    HeadlessImGui gui;
    SheetInScope unloaded{nullptr, SheetData{"textures/somewhere.png", glm::ivec2(16)}};
    FrameAnimationData nothing;
    TileData tile;

    REQUIRE(drawnBy(gui, [&] { drawAnimationPreview(unloaded, nothing); }).vertices == 0);
    REQUIRE(drawnBy(gui, [&] { drawTilePreview(unloaded, 0, tile); }).vertices == 0);
}

#endif
