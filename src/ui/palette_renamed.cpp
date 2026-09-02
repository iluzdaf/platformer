#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include "ui/palette_renamed.hpp"
#include "tile_map/tile_palette.hpp"

std::optional<std::string> whyNotARename(
    const TilePalettes &tilePalettes,
    const std::string &from,
    const std::string &to)
{
    if (to.empty())
        return "a palette needs a name";

    if (to == from)
        return std::nullopt;

    if (tilePalettes.contains(to))
        return "there is already a palette called \"" + to + "\"";

    return std::nullopt;
}

std::string aNameNobodyHasTaken(const TilePalettes &tilePalettes)
{
    for (std::size_t suffix = tilePalettes.size() + 1;; ++suffix)
    {
        std::string name = "palette " + std::to_string(suffix);
        if (!tilePalettes.contains(name))
            return name;
    }
}

void rememberRename(
    std::map<std::string, std::string> &renames,
    const std::string &from,
    const std::string &to)
{
    std::string onDisk = from;
    for (const auto &[was, is] : renames)
        if (is == from)
            onDisk = was;

    if (onDisk == to)
        renames.erase(onDisk);
    else
        renames.insert_or_assign(onDisk, to);
}
