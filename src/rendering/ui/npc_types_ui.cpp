#include <imgui.h>
#include "rendering/ui/npc_types_ui.hpp"
#include "rendering/ui/data_inspector.hpp"
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
