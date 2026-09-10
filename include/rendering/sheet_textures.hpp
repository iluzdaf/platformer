#pragma once

#include <stdexcept>
#include <string>
#include "assets/every_sheet_fitting_in.hpp"
#include "assets/sheet_data.hpp"
#include "rendering/texture2d.hpp"
#include "rendering/texture_cache.hpp"

struct ActorData;
struct PickupData;
struct ScoreIconData;
struct HealthIconData;
struct TilePaletteData;
struct GameData;

void checkFits(const ActorData &actor, const std::string &whose, int width, int height);

void checkFits(const PickupData &pickup, const std::string &whose, int width, int height);

void checkFits(const TilePaletteData &palette, const std::string &whose, int width, int height);

void checkFits(const ScoreIconData &icon, const std::string &whose, int width, int height);

void checkFits(const HealthIconData &icon, const std::string &whose, int width, int height);

template <class T>
void warmAndCheck(TextureCache &textures, const T &value, const std::string &whose)
{
    const SheetData &sheet = theSheetIn(value);
    if (sheet.texture.path.empty())
        throw std::runtime_error("No sheet is named for " + whose);

    textures.warm(sheet.texture.path);
    const Texture2D &texture = textures.get(sheet.texture.path);
    checkFits(
        value, whose, static_cast<int>(texture.getWidth()), static_cast<int>(texture.getHeight()));
}

void warmEverySheetIn(TextureCache &textures, const GameData &gameData);

void checkEverythingFits(TextureCache &textures, const GameData &gameData);
