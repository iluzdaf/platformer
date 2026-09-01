#include <optional>
#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include "test_helpers/headless_imgui.hpp"
#include "tile_map/tile_collider_data.hpp"
#include "tile_map/tile_data.hpp"
#include "ui/data_inspector.hpp"

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
