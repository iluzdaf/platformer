#include <imgui.h>
#include "rendering/ui/game_data_ui.hpp"
#include "rendering/ui/editor_section.hpp"
#include "rendering/ui/data_inspector.hpp"
#include "game/game_data.hpp"

namespace
{
    template <class T, class Save> void drawSaveable(T &value, Save &&save)
    {
        if (ImGui::Button("save"))
            save(value);

        ImGui::Separator();
        inspector::drawFields(value);
    }
}

void GameDataUi::draw(EditorSection section, GameData &gameData)
{
    switch (section)
    {
    case EditorSection::Game:
        drawSaveable(gameData.settings, saveGameSettings);
        break;

    case EditorSection::Camera:
        ImGui::Separator();
        drawSaveable(gameData.cameraData, saveCameraData);
        break;

    case EditorSection::Player:
        ImGui::Separator();
        drawSaveable(gameData.playerData, savePlayerData);
        break;

    case EditorSection::NpcTypes:
        if (ImGui::Button("save"))
            saveNpcData(gameData.npcData);

        ImGui::Separator();
        inspector::draw("types", gameData.npcData);
        break;

    case EditorSection::TilePalettes:
        if (ImGui::Button("save"))
            saveTilePalettes(gameData.tilePalettes);

        ImGui::Separator();
        inspector::draw("palettes", gameData.tilePalettes);
        break;

    default:
        break;
    }
}
