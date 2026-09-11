#include <catch2/catch_test_macros.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/sheet_data.hpp"
#include "game/health_icon_data.hpp"
#include "ui/data_inspector.hpp"
#include "ui/health_icon_field.hpp"
#include "ui/inspector_fields.hpp"

TEST_CASE("A health icon draws itself rather than falling through", "[HealthIconField]")
{
    STATIC_REQUIRE(inspector::HasCustomField<HealthIconData>);
}

#ifndef SKIP_OPENGL_TESTS

#include <imgui_internal.h>
#include "helpers/headless_imgui.hpp"
#include "helpers/made_sheet.hpp"
#include "helpers/pictures_drawn.hpp"
#include "rendering/texture2d.hpp"
#include "ui/in_scope.hpp"
#include "ui/sheet_in_scope.hpp"
#include "ui/tile_picker.hpp"
#include <cstdint>
#include <set>
#include <string>
#include <tuple>
#include "game/game_data.hpp"
#include "rendering/texture_cache.hpp"
#include "ui/editor_commands.hpp"
#include "ui/game_settings_ui.hpp"

namespace
{
    int picturesOfTheSheetIn(HeadlessImGui &gui, HealthIconData &icon, bool withSheet)
    {
        Texture2D sheet = aSheetOf(7, 6);
        SheetInScope offering{&sheet, icon.sheet};

        return picturesWideDrawn(
            gui,
            TilePickerCellSize,
            [&]
            {
                ImGui::TreeNodeSetOpen(ImGui::GetID("healthIcon"), true);

                if (!withSheet)
                {
                    inspector::draw("healthIcon", icon);
                    return;
                }

                InScope showing(offering);
                inspector::draw("healthIcon", icon);
            });
    }
}

TEST_CASE("Both marks are pictures to pick when a sheet is in scope", "[HealthIconField]")
{
    HeadlessImGui gui;
    HealthIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3, 4};

    REQUIRE(picturesOfTheSheetIn(gui, icon, true) == 2);
    REQUIRE(picturesOfTheSheetIn(gui, icon, false) == 0);
}

TEST_CASE("Each icon is drawn under its own sheet", "[HealthIconField]")
{
    HeadlessImGui gui;
    GameData gameData = loadGameData();
    const std::string &score = gameData.settings.scoreIcon.sheet.texture.path;
    const std::string &health = gameData.settings.healthIcon.sheet.texture.path;
    REQUIRE(score != health);

    TextureCache textures;
    textures.warm(score);
    textures.warm(health);
    EditorCommands commands;
    GameSettingsUi settingsUi;
    std::ignore = settingsUi.unsavedSince(gameData);

    std::set<ImTextureID> drawn = sheetsDrawnWhile(
        gui,
        [&]
        {
            ImGui::TreeNodeSetOpen(ImGui::GetID("scoreIcon"), true);
            ImGui::TreeNodeSetOpen(ImGui::GetID("healthIcon"), true);
            settingsUi.draw(gameData, textures, commands);
        });

    REQUIRE(drawn.contains((ImTextureID)(intptr_t)textures.get(score).getTextureID()));
    REQUIRE(drawn.contains((ImTextureID)(intptr_t)textures.get(health).getTextureID()));
}

TEST_CASE("What a health icon shows keeps what it was when nothing is picked", "[HealthIconField]")
{
    HeadlessImGui gui;
    HealthIconData icon{SheetData{"textures/somewhere.png", glm::ivec2(16)}, 3, 4};

    picturesOfTheSheetIn(gui, icon, true);

    REQUIRE(icon.full == 3);
    REQUIRE(icon.spent == 4);
}

#endif
