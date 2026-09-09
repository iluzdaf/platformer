#include <string>
#include <vector>
#include "rendering/sheet_textures.hpp"
#include "rendering/texture_cache.hpp"
#include "rendering/tile_set_fit.hpp"
#include "rendering/frames_fit.hpp"
#include "tile_map/tile_palette_data.hpp"
#include "actor/actor_data.hpp"
#include "actor/actor_animation_data.hpp"
#include "pickups/pickup_data.hpp"
#include "animations/frame_animation_data.hpp"
#include "game/score_icon_data.hpp"
#include "game/health_icon_data.hpp"
#include "game/game_data.hpp"
#include "assets/sheet_data.hpp"
#include "assets/every_sheet_in.hpp"
#include "assets/every_sheet_fitting_in.hpp"

void checkFits(const ActorData &actor, const std::string &whose, int width, int height)
{
    for (const auto &[name, clip] : actor.animationData.clips)
    {
        std::string named = whose;
        named += " ";
        named += name;
        checkFramesFit(clip.frames, actor.sheet, named, width, height);
        checkCuesFit(clip, named);
    }
}

void checkFits(const PickupData &pickup, const std::string &whose, int width, int height)
{
    checkFramesFit(pickup.animationData.frames, pickup.sheet, whose, width, height);
    checkCuesFit(pickup.animationData, whose);
}

void checkFits(const TilePaletteData &palette, const std::string &whose, int width, int height)
{
    checkTileSetFits(palette, whose, width, height);
}

void checkFits(const ScoreIconData &icon, const std::string &whose, int width, int height)
{
    checkFramesFit({icon.frame}, icon.sheet, whose, width, height);
}

void checkFits(const HealthIconData &icon, const std::string &whose, int width, int height)
{
    checkFramesFit({icon.full, icon.spent}, icon.sheet, whose, width, height);
}

void warmEverySheetIn(TextureCache &textures, const GameData &gameData)
{
    everySheetIn(
        gameData,
        [&textures](const SheetData &sheet)
        {
            if (!sheet.texture.empty())
                textures.warm(sheet.texture);
        });
}

void checkEverythingFits(TextureCache &textures, const GameData &gameData)
{
    everySheetFittingIn(
        gameData,
        std::string{},
        [&textures](const auto &value, const std::string &whose)
        { warmAndCheck(textures, value, whose); });
}
