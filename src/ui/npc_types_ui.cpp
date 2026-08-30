#include <imgui.h>
#include "ui/npc_types_ui.hpp"
#include "ui/data_inspector.hpp"
#include "game/game_data.hpp"

void NpcTypesUi::draw(GameData &gameData)
{
    saveable.drawControls("npcs", gameData.npcData, saveNpcData);
    ImGui::Separator();
    inspector::draw("types", gameData.npcData);
}

void NpcTypesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
