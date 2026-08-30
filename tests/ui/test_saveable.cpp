#include <string>
#include <tuple>
#include <catch2/catch_test_macros.hpp>
#include <glaze/glaze.hpp>
#include "cameras/camera2d_data.hpp"
#include "test_helpers/headless_imgui.hpp"
#include "ui/saveable.hpp"

namespace
{
    std::string jsonOf(const Camera2DData &data)
    {
        std::string json;
        std::ignore = glz::write_json(data, json);
        return json;
    }

    void drawOnce(HeadlessImGui &gui, Saveable &saveable, Camera2DData &data)
    {
        gui.frame([&] { saveable.drawControls("camera", data, [](const Camera2DData &) {}); });
    }
}

TEST_CASE("Saveable has nothing to save until a value changes", "[Saveable]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    drawOnce(gui, saveable, data);

    REQUIRE_FALSE(saveable.unsaved("camera", jsonOf(data)));
}

TEST_CASE("Saveable notices a changed value", "[Saveable]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    drawOnce(gui, saveable, data);
    data.zoom += 1.0f;

    REQUIRE(saveable.unsaved("camera", jsonOf(data)));
}

TEST_CASE("Saveable does not treat drawing as saving", "[Saveable]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    drawOnce(gui, saveable, data);
    data.zoom += 1.0f;
    drawOnce(gui, saveable, data);

    REQUIRE(saveable.unsaved("camera", jsonOf(data)));
}

TEST_CASE("Saveable takes replaced values as the new baseline", "[Saveable]")
{
    HeadlessImGui gui;
    Saveable saveable;
    Camera2DData data;

    drawOnce(gui, saveable, data);
    data.zoom += 1.0f;
    saveable.valuesReplaced();
    drawOnce(gui, saveable, data);

    REQUIRE_FALSE(saveable.unsaved("camera", jsonOf(data)));
}

TEST_CASE("Saveable has nothing to say about a name it has never drawn", "[Saveable]")
{
    Saveable saveable;
    Camera2DData data;

    REQUIRE_FALSE(saveable.unsaved("camera", jsonOf(data)));
}
