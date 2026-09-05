#include <optional>
#include <string>
#include "ui/type_shown.hpp"
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "assets/sheet_data.hpp"
#include "actor/actor_data.hpp"

TypeShown addTypeTo(GameData &gameData, TypeShown::What what)
{
    if (what == TypeShown::What::Npc)
    {
        std::string name = aTypeNameNobodyHasTaken(gameData.npcData);
        gameData.npcData.insert({name, NpcData{}});

        return TypeShown{what, name};
    }

    std::string name = aTypeNameNobodyHasTaken(gameData.pickupData);
    gameData.pickupData.insert({name, PickupData{}});

    return TypeShown{what, name};
}

void removeTypeFrom(GameData &gameData, const TypeShown &showing)
{
    if (showing.what == TypeShown::What::Npc)
        gameData.npcData.erase(showing.name);
    else
        gameData.pickupData.erase(showing.name);
}

namespace
{
    std::optional<std::string> whyNot(const SheetData &sheet)
    {
        if (sheet.texture.empty())
            return "names no sheet to draw from";

        return std::nullopt;
    }
}

std::optional<std::string> whyATypeCannotBeSaved(const GameData &gameData, const TypeShown &type)
{
    if (type.what == TypeShown::What::Npc)
    {
        auto known = gameData.npcData.find(type.name);

        return known == gameData.npcData.end() ? std::nullopt
                                               : whyNot(known->second.actorData.sheet);
    }

    auto known = gameData.pickupData.find(type.name);

    return known == gameData.pickupData.end() ? std::nullopt : whyNot(known->second.sheet);
}

std::optional<std::string> typesNamingNoSheet(const GameData &gameData)
{
    std::string names;
    auto nameIfCannot = [&](TypeShown::What what, const std::string &name)
    {
        if (whyATypeCannotBeSaved(gameData, TypeShown{what, name}))
            names += (names.empty() ? "" : ", ") + name;
    };

    for (const auto &[name, npc] : gameData.npcData)
        nameIfCannot(TypeShown::What::Npc, name);

    for (const auto &[name, pickup] : gameData.pickupData)
        nameIfCannot(TypeShown::What::Pickup, name);

    if (names.empty())
        return std::nullopt;

    return names + " name no sheet to draw from";
}

const SheetData *sheetOf(const GameData &gameData, const TypeShown &showing)
{
    if (showing.what == TypeShown::What::Npc)
    {
        auto known = gameData.npcData.find(showing.name);

        return known == gameData.npcData.end() ? nullptr : &known->second.actorData.sheet;
    }

    auto known = gameData.pickupData.find(showing.name);

    return known == gameData.pickupData.end() ? nullptr : &known->second.sheet;
}
