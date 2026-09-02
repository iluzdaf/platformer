#pragma once

#include <map>
#include <string>

class TextureCache;
struct PlayerData;
struct NpcData;

void warmActorTextures(
    TextureCache &textures,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData);
