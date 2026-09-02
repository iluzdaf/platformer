#include <map>
#include <stdexcept>
#include <string>
#include "rendering/actor_textures.hpp"
#include "rendering/texture_cache.hpp"
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"

namespace
{
    void warmOne(TextureCache &textures, const ActorData &actorData, const std::string &whose)
    {
        if (actorData.sheet.texture.empty())
            throw std::runtime_error("No sprite sheet is named for " + whose);

        textures.warm(actorData.sheet.texture);
    }
}

void warmActorTextures(
    TextureCache &textures,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData)
{
    warmOne(textures, playerData.actorData, "the player");

    for (const auto &[name, data] : npcData)
        warmOne(textures, data.actorData, "\"" + name + "\"");
}
