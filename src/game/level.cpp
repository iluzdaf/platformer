#include <algorithm>
#include <stdexcept>
#include "game/level.hpp"
#include "navigation/navigation_graph_builder.hpp"

namespace
{
    NavigationProfile profileOf(const ActorData &actorData)
    {
        return NavigationProfile{actorData.physicsBodyData.colliderSize};
    }
}

Level::Level(
    const TileMapData &tileMapData,
    const TilePalettes &tilePalettes,
    const PlayerData &playerData,
    const std::unordered_map<std::string, NpcData> &npcData)
    : tileMap(tileMapData, tilePalettes)
{
    addGraphFor(profileOf(playerData.actorData));

    for (const NpcSpawnData &spawn : tileMap.getNpcs())
    {
        auto npc = npcData.find(spawn.type);
        if (npc == npcData.end())
            throw std::runtime_error(
                "Unknown npc \"" + spawn.type + "\" in " + tileMap.getLevel());

        addGraphFor(profileOf(npc->second.actorData));
    }
}

void Level::addGraphFor(const NavigationProfile &profile)
{
    if (std::find(profiles.begin(), profiles.end(), profile) != profiles.end())
        return;

    profiles.push_back(profile);
    graphs.push_back(buildNavigationGraph(tileMap, profile));
}

const TileMap &Level::getTileMap() const
{
    return tileMap;
}

const NavigationGraph &Level::graphFor(const NavigationProfile &profile) const
{
    auto found = std::find(profiles.begin(), profiles.end(), profile);
    if (found == profiles.end())
        throw std::runtime_error("This level has no navigation graph for that actor");

    return graphs[static_cast<size_t>(std::distance(profiles.begin(), found))];
}
