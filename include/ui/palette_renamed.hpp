#pragma once

#include <map>
#include <optional>
#include <string>
#include "tile_map/tile_palette.hpp"

struct PaletteRenamed
{
    std::string from, to;
};

std::optional<std::string> whyNotARename(
    const TilePalettes &tilePalettes,
    const std::string &from,
    const std::string &to);

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes);

void rememberRename(
    std::map<std::string, std::string> &renames,
    const std::string &from,
    const std::string &to);
