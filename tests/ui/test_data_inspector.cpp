#include <optional>
#include <string_view>
#include <cstddef>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <glaze/glaze.hpp>
#include "animations/frame_animation_data.hpp"
#include "animations/sprite_animation_data.hpp"
#include "animations/tile_animation_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "tile_map/tile_index.hpp"
#include "tile_map/tile_set.hpp"
#include "tile_map/tile_pickup_data.hpp"
#include "ui/data_inspector.hpp"
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/tile_field_context.hpp"

namespace
{
    int itemsSharingTheIdOf(HeadlessImGui &gui, const char *fieldName, TileData &tileData)
    {
        gui.frame([&] { inspector::drawFields(tileData); });

        int sharing = 0;
        gui.frame(
            [&]
            {
                ImGui::GetCurrentContext()->HoveredIdPreviousFrame = ImGui::GetID(fieldName);
                inspector::drawFields(tileData);
                sharing = ImGui::GetCurrentContext()->HoveredIdPreviousFrameItemCount;
            });

        return sharing;
    }
}

TEST_CASE("An engaged optional does not lend its fields the checkbox's id", "[DataInspector]")
{
    HeadlessImGui gui;
    TileData tileData;
    tileData.collider = TileColliderData{};

    REQUIRE(itemsSharingTheIdOf(gui, "collider", tileData) == 1);
}

TEST_CASE("An optional nobody engaged draws only its checkbox", "[DataInspector]")
{
    HeadlessImGui gui;
    TileData tileData;

    REQUIRE(itemsSharingTheIdOf(gui, "collider", tileData) == 1);
}

namespace inspector_test
{
    struct Wrapped
    {
        TileColliderData collider;
    };

    struct Drawn
    {
        int value = 0;
    };

    struct Holder
    {
        TileColliderData collider;
    };

    inline int drawnByItsOwnField = 0;
    inline int holderDrawnByItsOwnField = 0;

    inline inspector::Edited drawCustomField(std::string_view name, Drawn &value)
    {
        ++drawnByItsOwnField;
        return inspector::drawNamed(name, value.value);
    }

    inline inspector::Edited drawCustomField(std::string_view name, Holder &value)
    {
        ++holderDrawnByItsOwnField;
        return inspector::draw(name, value.collider.offset);
    }
}

using namespace inspector_test;

TEST_CASE("A type with a field of its own is drawn by it", "[DataInspector]")
{
    STATIC_REQUIRE(inspector::HasCustomField<Drawn>);

    HeadlessImGui gui;
    Drawn drawn;
    drawnByItsOwnField = 0;

    gui.frame([&] { inspector::draw("drawn", drawn); });

    REQUIRE(drawnByItsOwnField == 1);
}

TEST_CASE("A tile index falls back to a number when no sheet is offered", "[DataInspector]")
{
    REQUIRE(tilesOnOffer() == nullptr);

    HeadlessImGui gui;
    TilePickupData pickup;
    pickup.replaceIndex = 5;

    gui.frame([&] { inspector::drawFields(pickup); });

    REQUIRE(pickup.replaceIndex.value == 5);
}

TEST_CASE("What is on offer is put back when the scope ends", "[DataInspector]")
{
    TileFieldContext offering;
    offering.tileSet.tileSize = 8;

    REQUIRE(tilesOnOffer() == nullptr);
    {
        ShowingTilesFrom showing(offering);
        REQUIRE(tilesOnOffer() == &offering);
        REQUIRE(tilesOnOffer()->tileSet.tileSize == 8);

        TileFieldContext inner;
        {
            ShowingTilesFrom nested(inner);
            REQUIRE(tilesOnOffer() == &inner);
        }
        REQUIRE(tilesOnOffer() == &offering);
    }
    REQUIRE(tilesOnOffer() == nullptr);
}

TEST_CASE("A tile index draws itself rather than falling through", "[DataInspector]")
{
    STATIC_REQUIRE(inspector::HasCustomField<TileIndex>);
    STATIC_REQUIRE_FALSE(glz::reflectable<TileIndex>);
    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<TileIndex>());

    HeadlessImGui gui;
    TilePickupData pickup;
    pickup.replaceIndex = 12;

    gui.frame([&] { inspector::drawFields(pickup); });

    REQUIRE(pickup.replaceIndex.value == 12);
}

TEST_CASE("A type without one still reaches the reflection path", "[DataInspector]")
{
    STATIC_REQUIRE_FALSE(inspector::HasCustomField<TileSet>);
    STATIC_REQUIRE_FALSE(inspector::HasCustomField<Camera2DData>);
    STATIC_REQUIRE_FALSE(inspector::HasCustomField<int>);

    HeadlessImGui gui;
    TileSet tileSet{"textures/tile_set.png", 16};

    gui.frame([&] { inspector::draw("tileSet", tileSet); });

    REQUIRE(tileSet.tileSize == 16);
}

TEST_CASE("The types that draw themselves say so", "[DataInspector]")
{
    STATIC_REQUIRE(inspector::HasCustomField<TileIndex>);
    STATIC_REQUIRE(inspector::HasCustomField<TileColliderData>);
    STATIC_REQUIRE(inspector::HasCustomField<TileAnimationData>);
}

TEST_CASE("A field of its own wins over being drawn through", "[DataInspector]")
{
    STATIC_REQUIRE(inspector::wrapsOneStruct<Wrapped>());
    STATIC_REQUIRE(inspector::wrapsOneStruct<Holder>());
    STATIC_REQUIRE(inspector::HasCustomField<Holder>);

    HeadlessImGui gui;
    Holder holder;
    holderDrawnByItsOwnField = 0;

    gui.frame([&] { inspector::draw("holder", holder); });

    REQUIRE(holderDrawnByItsOwnField == 1);
}

TEST_CASE("Only a struct wrapping one struct is drawn through", "[DataInspector]")
{
    STATIC_REQUIRE(inspector::wrapsOneStruct<TileAnimationData>());

    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<SpriteAnimationData>());
    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<Camera2DData>());
    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<TileColliderData>());
    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<FrameAnimationData>());
    STATIC_REQUIRE_FALSE(inspector::wrapsOneStruct<int>());
}

TEST_CASE("What a wrapper held is still drawn", "[DataInspector]")
{
    HeadlessImGui gui;
    TileData tileData;
    tileData.animationData = TileAnimationData{{{4, 5, 6}, 0.25f}};

    gui.frame([&] { inspector::drawFields(tileData); });
    gui.frame([&] { inspector::drawFields(tileData); });

    REQUIRE(tileData.animationData->frameAnimationData.frames == std::vector<int>{4, 5, 6});
    REQUIRE(tileData.animationData->frameAnimationData.frameDuration == 0.25f);
}

TEST_CASE("An engaged optional still keeps the value it was given", "[DataInspector]")
{
    HeadlessImGui gui;
    TileData tileData;
    tileData.collider = TileColliderData{glm::vec2(2.0f, 3.0f), glm::vec2(4.0f, 5.0f)};

    gui.frame([&] { inspector::drawFields(tileData); });

    REQUIRE(tileData.collider.has_value());
    REQUIRE(tileData.collider->offset == glm::vec2(2.0f, 3.0f));
    REQUIRE(tileData.collider->size == glm::vec2(4.0f, 5.0f));
}

namespace
{
    ImVec2 centreOf(ImVec2 low, ImVec2 high)
    {
        return ImVec2((low.x + high.x) * 0.5f, (low.y + high.y) * 0.5f);
    }

    struct Rows
    {
        ImVec2 add;
        float top = 0.0f, x = 0.0f;

        ImVec2 takeAway(std::size_t row) const
        {
            return ImVec2(
                x,
                top + ImGui::GetTextLineHeightWithSpacing() +
                    static_cast<float>(row) * ImGui::GetFrameHeightWithSpacing() +
                    ImGui::GetFrameHeight() * 0.5f);
        }
    };

    Rows rowsOf(HeadlessImGui &gui, std::vector<int> &frames)
    {
        Rows found;
        auto drawOpen = [&]
        {
            found.top = ImGui::GetCursorScreenPos().y;
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("frames", frames);
            found.add = centreOf(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            found.x = found.add.x;
        };

        gui.frame(drawOpen);
        gui.frame(drawOpen);

        return found;
    }
}

TEST_CASE("A row can be added to a list", "[DataInspector]")
{
    HeadlessImGui gui;
    std::vector<int> frames{1, 2, 3};

    Rows rows = rowsOf(gui, frames);
    gui.clickAt(
        rows.add,
        [&]
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("frames", frames);
        });

    REQUIRE(frames == std::vector<int>{1, 2, 3, 0});
}

TEST_CASE("A row can be taken away and the rest keep their order", "[DataInspector]")
{
    HeadlessImGui gui;
    std::vector<int> frames{4, 5, 6};

    Rows rows = rowsOf(gui, frames);
    gui.clickAt(
        rows.takeAway(1),
        [&]
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
            inspector::draw("frames", frames);
        });

    REQUIRE(frames == std::vector<int>{4, 6});
}

TEST_CASE("A list nobody clicks keeps what it had", "[DataInspector]")
{
    HeadlessImGui gui;
    std::vector<int> frames{7, 8};

    auto drawOpen = [&]
    {
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        inspector::draw("frames", frames);
    };

    gui.frame(drawOpen);
    gui.frame(drawOpen);

    REQUIRE(frames == std::vector<int>{7, 8});
}
