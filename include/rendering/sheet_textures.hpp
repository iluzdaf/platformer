#pragma once

#include <map>
#include <string>
#include "tile_map/tile_palette.hpp"

class TextureCache;
struct PlayerData;
struct NpcData;
struct PickupData;

void warmTileSets(TextureCache &textures, const TilePalettes &tilePalettes);

void warmActorTextures(
    TextureCache &textures,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData);

void warmPickupTextures(
    TextureCache &textures,
    const std::map<std::string, PickupData> &pickupData);
