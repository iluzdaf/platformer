#include <optional>
#include <stdexcept>
#include <string>
#include "ui/type_shown.hpp"
#include "game/game_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "player/player_data.hpp"
#include "assets/sheet_data.hpp"
#include "actor/actor_data.hpp"

TypeShown thePlayer()
{
    return TypeShown{TypeShown::What::Player, "player"};
}

TypeShown addTypeTo(GameData &gameData, TypeShown::What what)
{
    if (what == TypeShown::What::Npc)
    {
        std::string name = aTypeNameNobodyHasTaken(gameData.npcData);
        gameData.npcData.insert({name, NpcData{}});
        return TypeShown{what, name};
    }

    if (what == TypeShown::What::Player)
        throw std::runtime_error("There is one player, and the cast already has them");

    std::string name = aTypeNameNobodyHasTaken(gameData.pickupData);
    gameData.pickupData.insert({name, PickupData{}});
    return TypeShown{what, name};
}

void removeTypeFrom(GameData &gameData, const TypeShown &showing)
{
    switch (showing.what)
    {
    case TypeShown::What::Npc:
        gameData.npcData.erase(showing.name);
        break;

    case TypeShown::What::Pickup:
        gameData.pickupData.erase(showing.name);
        break;

    case TypeShown::What::Player:
        throw std::runtime_error("The player cannot leave the cast");
    }
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
    const SheetData *sheet = sheetOf(gameData, type);

    return sheet ? whyNot(*sheet) : std::nullopt;
}

std::optional<std::string> typesNamingNoSheet(const GameData &gameData)
{
    std::string names;
    auto nameIfCannot = [&](const TypeShown &type)
    {
        if (whyATypeCannotBeSaved(gameData, type))
            names += (names.empty() ? "" : ", ") + type.name;
    };

    nameIfCannot(thePlayer());
    for (const auto &[name, npc] : gameData.npcData)
        nameIfCannot(TypeShown{TypeShown::What::Npc, name});
    for (const auto &[name, pickup] : gameData.pickupData)
        nameIfCannot(TypeShown{TypeShown::What::Pickup, name});

    if (names.empty())
        return std::nullopt;

    return names + " name no sheet to draw from";
}

const SheetData *sheetOf(const GameData &gameData, const TypeShown &showing)
{
    switch (showing.what)
    {
    case TypeShown::What::Npc: {
        auto known = gameData.npcData.find(showing.name);
        return known == gameData.npcData.end() ? nullptr : &known->second.actorData.sheet;
    }

    case TypeShown::What::Pickup: {
        auto known = gameData.pickupData.find(showing.name);
        return known == gameData.pickupData.end() ? nullptr : &known->second.sheet;
    }

    case TypeShown::What::Player:
        return &gameData.playerData.actorData.sheet;
    }

    return nullptr;
}
