#include <catch2/catch_test_macros.hpp>
#include "cameras/camera2d_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"

namespace
{
    void drawOnce(HeadlessImGui &gui, Saveable &saveable, Camera2DData &data)
    {
        gui.frame([&] { drawSaveControls(saveable, "camera", data, [](const Camera2DData &) {}); });
    }
}

TEST_CASE("Drawing the controls records what was on disk", "[SaveControls]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    REQUIRE(saveable.lastSeen("camera").empty());
    drawOnce(gui, saveable, data);

    REQUIRE(saveable.lastSeen("camera") == asJson(data));
}

TEST_CASE("Drawing the controls is not saving", "[SaveControls]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    drawOnce(gui, saveable, data);
    data.zoom += 1.0f;
    drawOnce(gui, saveable, data);

    REQUIRE(saveable.unsaved("camera", asJson(data)));
}
