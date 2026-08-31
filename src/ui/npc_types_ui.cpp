#include <imgui.h>
#include "ui/npc_types_ui.hpp"
#include "ui/save_controls.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "game/game_data.hpp"

void NpcTypesUi::draw(GameData &gameData)
{
    drawSaveControls(saveable, "npcs", gameData.npcData, saveNpcData);
    ImGui::Separator();
    inspector::draw("types", gameData.npcData);
}

bool NpcTypesUi::hasUnsavedChanges(const GameData &gameData) const
{
    return saveable.unsaved("npcs", asJson(gameData.npcData));
}

void NpcTypesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
