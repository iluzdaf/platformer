#pragma once

#include <map>
#include <string>
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "tile_map/tile_palette_data.hpp"

inline const TilePalettes &shippedPalettes()
{
    static const TilePalettes palettes = [] { return loadGameData().tilePalettes; }();
    return palettes;
}

inline const std::string &shippedPaletteName()
{
    return shippedPalettes().begin()->first;
}

inline const std::map<std::string, NpcData> &shippedNpcData()
{
    static const std::map<std::string, NpcData> npcData = [] { return loadGameData().npcData; }();
    return npcData;
}

inline const std::map<std::string, PickupData> &shippedPickupData()
{
    static const std::map<std::string, PickupData> pickupData = []
    { return loadGameData().pickupData; }();
    return pickupData;
}
