#include <map>
#include <stdexcept>
#include <string>
#include "rendering/sheet_textures.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/tile_set_fit.hpp"
#include "rendering/frames_fit.hpp"
#include "tile_map/tile_palette.hpp"
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "assets/sheet.hpp"

namespace
{
    std::string quoted(const std::string &name)
    {
        return "\"" + name + "\"";
    }

    void warmOne(TextureCache &textures, const Sheet &sheet, const std::string &whose)
    {
        if (sheet.texture.empty())
            throw std::runtime_error("No sheet is named for " + whose);

        textures.warm(sheet.texture);
    }
}

void warmActorTextures(
    TextureCache &textures,
    const PlayerData &playerData,
    const std::map<std::string, NpcData> &npcData)
{
    warmOne(textures, playerData.actorData.sheet, "the player");

    for (const auto &[name, data] : npcData)
        warmOne(textures, data.actorData.sheet, quoted(name));
}

void warmPickupTextures(TextureCache &textures, const std::map<std::string, PickupData> &pickupData)
{
    for (const auto &[name, data] : pickupData)
    {
        warmOne(textures, data.sheet, quoted(name));

        const Texture2D &texture = textures.get(data.sheet.texture);
        checkFramesFit(
            data.animationData,
            data.sheet,
            quoted(name),
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()));
    }
}

void warmTileSets(TextureCache &textures, const TilePalettes &tilePalettes)
{
    for (const auto &[paletteName, palette] : tilePalettes)
    {
        warmOne(textures, palette.tileSet, quoted(paletteName));

        const Texture2D &texture = textures.get(palette.tileSet.texture);
        checkTileSetFits(
            palette,
            paletteName,
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()));
    }
}
