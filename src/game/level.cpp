#include <algorithm>
#include <stdexcept>
#include "game/level.hpp"
#include "navigation/navigation_graph_builder.hpp"

namespace
{
    NavigationProfile profileOf(const NpcData &npcData)
    {
        return NavigationProfile{npcData.actorData.physicsBodyData.colliderSize};
    }
}

Level::Level(
    const TileMapData &tileMapData,
    const TilePalettes &tilePalettes,
    const std::unordered_map<std::string, NpcData> &npcData)
    : tileMap(tileMapData, tilePalettes)
{
    std::vector<NavigationProfile> profiles;

    for (const NpcSpawnData &spawn : tileMap.getNpcs())
    {
        auto npc = npcData.find(spawn.type);
        if (npc == npcData.end())
            throw std::runtime_error("Unknown npc \"" + spawn.type + "\"");

        NavigationProfile profile = profileOf(npc->second);
        auto existing = std::find(profiles.begin(), profiles.end(), profile);

        if (existing == profiles.end())
        {
            profiles.push_back(profile);
            graphs.push_back(buildNavigationGraph(tileMap, profile));
            existing = profiles.end() - 1;
        }

        graphByNpcType[spawn.type] =
            static_cast<size_t>(std::distance(profiles.begin(), existing));
    }
}

const TileMap &Level::getTileMap() const
{
    return tileMap;
}

const NavigationGraph &Level::graphFor(const std::string &npcType) const
{
    auto graph = graphByNpcType.find(npcType);
    if (graph == graphByNpcType.end())
        throw std::runtime_error("No npc \"" + npcType + "\" in this level");

    return graphs[graph->second];
}
