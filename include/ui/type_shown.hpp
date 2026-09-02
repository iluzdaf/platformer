#pragma once

#include <map>
#include <optional>
#include <string>

struct TypeShown
{
    enum class What
    {
        Npc,
        Pickup
    };

    What what = What::Npc;
    std::string name;

    bool operator==(const TypeShown &) const = default;
};

struct GameData;

TypeShown addTypeTo(GameData &gameData, TypeShown::What what);

void removeTypeFrom(GameData &gameData, const TypeShown &showing);

std::optional<std::string> whyATypeCannotBeSaved(const GameData &gameData, const TypeShown &type);

std::optional<std::string> typesNamingNoSheet(const GameData &gameData);

template <class T> std::string aTypeNameNobodyHasTaken(const std::map<std::string, T> &types)
{
    std::string name = "new";
    for (int suffix = 2; types.contains(name); ++suffix)
        name = "new " + std::to_string(suffix);

    return name;
}
