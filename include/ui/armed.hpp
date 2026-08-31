#pragma once

#include <cstddef>
#include <optional>
#include <variant>

inline constexpr unsigned int ArmedColour = 0xFF00FFFF;

struct PaintTile
{
    int tileIndex = 0;

    bool operator==(const PaintTile &) const = default;
};

struct PickTile
{
    enum class For
    {
        PlayerStart,
        NpcSpawn,
        PatrolFrom,
        PatrolTo
    };

    For what = For::PlayerStart;
    std::size_t npcIndex = 0;

    bool operator==(const PickTile &) const = default;
};

using Armed = std::variant<PaintTile, PickTile>;

inline std::optional<int> paintedTile(const std::optional<Armed> &armed)
{
    if (!armed)
        return std::nullopt;

    const PaintTile *painting = std::get_if<PaintTile>(&*armed);

    return painting ? std::optional<int>(painting->tileIndex) : std::nullopt;
}
