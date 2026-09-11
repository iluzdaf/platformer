#include <catch2/catch_test_macros.hpp>
#include <imgui.h>
#include "ui/panel_placement.hpp"

namespace
{
    constexpr ImVec2 ADisplay{800.0f, 600.0f};
}

TEST_CASE("A panel is as tall as the display, and no taller", "[PanelPlacement]")
{
    PanelPlacement placed = panelPinnedRight(ADisplay, 280.0f);

    REQUIRE(placed.position.y == 0.0f);
    REQUIRE(placed.smallest.y == ADisplay.y);
    REQUIRE(placed.largest.y == ADisplay.y);
}

TEST_CASE("A panel keeps its right edge against the display's", "[PanelPlacement]")
{
    REQUIRE(panelPinnedRight(ADisplay, 280.0f).position.x == ADisplay.x - 280.0f);
    REQUIRE(panelPinnedRight(ADisplay, 400.0f).position.x == ADisplay.x - 400.0f);
}

TEST_CASE("A panel dragged too narrow stops at the narrowest", "[PanelPlacement]")
{
    PanelPlacement placed = panelPinnedRight(ADisplay, 10.0f);

    REQUIRE(placed.position.x == ADisplay.x - NarrowestPanel);
    REQUIRE(placed.smallest.x == NarrowestPanel);
}

TEST_CASE("A panel cannot be dragged over the whole display", "[PanelPlacement]")
{
    PanelPlacement placed = panelPinnedRight(ADisplay, ADisplay.x);

    REQUIRE(placed.largest.x == ADisplay.x * 0.8f);
    REQUIRE(placed.position.x == ADisplay.x - placed.largest.x);
}

TEST_CASE("A display narrower than the narrowest panel still shows it", "[PanelPlacement]")
{
    ImVec2 small{120.0f, 90.0f};
    PanelPlacement placed = panelPinnedRight(small, 280.0f);

    REQUIRE(placed.position.x == 0.0f);
    REQUIRE(placed.largest.x == NarrowestPanel);
    REQUIRE(placed.smallest.x == NarrowestPanel);
}
