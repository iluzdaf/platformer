#include <glaze/glaze.hpp>
#include <imgui.h>
#include "ui/npc_types_ui.hpp"
#include "ui/saveable.hpp"
#include "ui/data_inspector.hpp"
#include "game/game_data.hpp"

void NpcTypesUi::draw(GameData &gameData)
{
    inspector::draw("types", gameData.npcData);
}
void NpcTypesUi::revert(GameData &gameData)
{
    revertTo(saveable, "npcs", gameData.npcData);
}

void NpcTypesUi::save(GameData &gameData)
{
    saveNpcData(gameData.npcData);
    saveable.saved("npcs", asJson(gameData.npcData));
}

bool NpcTypesUi::unsavedSince(const GameData &gameData)
{
    return saveable.unsavedSince("npcs", asJson(gameData.npcData));
}

void NpcTypesUi::valuesReplaced()
{
    saveable.valuesReplaced();
}
