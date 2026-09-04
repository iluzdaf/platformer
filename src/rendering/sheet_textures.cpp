#include <map>
#include <stdexcept>
#include <string>
#include "rendering/sheet_textures.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/tile_set_fit.hpp"
#include "rendering/frames_fit.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "actor/actor_data.hpp"
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/score_icon_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/actor_animation_data.hpp"
#include <optional>
#include <vector>
#include "assets/sheet_data.hpp"

namespace
{
    std::string quoted(const std::string &name)
    {
        return "\"" + name + "\"";
    }

    void framesFitting(
        TextureCache &textures,
        const SheetData &sheet,
        const std::vector<int> &frames,
        const std::string &whose)
    {
        const Texture2D &texture = textures.get(sheet.texture);
        checkFramesFit(
            frames,
            sheet,
            whose,
            static_cast<int>(texture.getWidth()),
            static_cast<int>(texture.getHeight()));
    }

    void everyAnimationOf(
        TextureCache &textures,
        const ActorData &actorData,
        const std::string &whose)
    {
        const ActorAnimationData &animations = actorData.animationData;
        framesFitting(textures, actorData.sheet, animations.idle.frames, whose);

        for (const std::optional<FrameAnimationData> &animation :
             {animations.walk,
              animations.dash,
              animations.jump,
              animations.fall,
              animations.wallSlide})
            if (animation)
                framesFitting(textures, actorData.sheet, animation->frames, whose);
    }

    void warmOne(TextureCache &textures, const SheetData &sheet, const std::string &whose)
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
    everyAnimationOf(textures, playerData.actorData, "the player");

    for (const auto &[name, data] : npcData)
    {
        warmOne(textures, data.actorData.sheet, quoted(name));
        everyAnimationOf(textures, data.actorData, quoted(name));
    }
}

void warmPickupTextures(TextureCache &textures, const std::map<std::string, PickupData> &pickupData)
{
    for (const auto &[name, data] : pickupData)
    {
        warmOne(textures, data.sheet, quoted(name));

        framesFitting(textures, data.sheet, data.animationData.frames, quoted(name));
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

void warmScoreIcon(TextureCache &textures, const ScoreIconData &scoreIcon)
{
    warmOne(textures, scoreIcon.sheet, "the score");

    framesFitting(textures, scoreIcon.sheet, {scoreIcon.frame}, "the score");
}
