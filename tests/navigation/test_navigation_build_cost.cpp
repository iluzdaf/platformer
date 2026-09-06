#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <map>
#include <string>
#include "game/level.hpp"
#include "game/game_data.hpp"
#include "game/level_data_file.hpp"
#include "navigation/named_navigation_graph.hpp"
#include "navigation/navigation_build_report.hpp"
#include "navigation/navigation_graph.hpp"
#include "test_helpers/asset_path.hpp"
#include "test_helpers/test_tile_map_utils.hpp"

namespace
{
    std::map<std::string, NavigationBuildReport> costOfBuilding(const std::string &levelPath)
    {
        Level level(
            readLevelData(levelPath),
            shippedPalettes(),
            loadGameData().playerData,
            shippedNpcData(),
            shippedPickupData());

        std::map<std::string, NavigationBuildReport> costs;
        for (const NamedNavigationGraph &graph : level.getGraphs())
            costs[graph.name] = graph.graph.builtWith();
        return costs;
    }
}

TEST_CASE("No simulated jump in a shipped level runs to its cap", "[Navigation][Cost]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        for (const auto &[name, report] : costOfBuilding(entry.path().string()))
        {
            INFO(entry.path().filename().string() << " " << name);
            REQUIRE(report.attemptsCapped == 0);
        }
    }
}

namespace
{
    struct Budget
    {
        int measured;
        const char *lastMoved;
    };

    // Physics steps the builder simulated for every graph of the level, measured on the
    // day of the last change that moved it. A load costing a third more than this fails;
    // so does one costing under two thirds, since doing less work is also worth knowing.
    const std::map<std::string, Budget> StepsToBuild{
        {"level1.json", {932, "2026-09-06, when the report was added"}},
        {"level2.json", {2411, "2026-09-06, when the report was added"}},
        {"level3.json", {2590, "2026-09-06, when the report was added"}},
        {"level4.json", {2164, "2026-09-06, when the report was added"}},
        {"level5.json", {6542, "2026-09-06, when the report was added"}},
        {"level6.json", {19268, "2026-09-06, when the report was added"}},
    };
}

TEST_CASE("Building a shipped level's graphs costs what it did last time", "[Navigation][Cost]")
{
    for (const auto &entry : std::filesystem::directory_iterator(assetPath("levels")))
    {
        if (entry.path().extension() != ".json")
            continue;

        std::string name = entry.path().filename().string();
        INFO(name << " has no budget in StepsToBuild; measure it and add one");
        REQUIRE(StepsToBuild.contains(name));

        int steps = 0;
        for (const auto &[graph, report] : costOfBuilding(entry.path().string()))
            steps += report.stepsSimulated;

        const Budget &budget = StepsToBuild.at(name);
        INFO(
            name << " simulated " << steps << " steps against " << budget.measured << " measured "
                 << budget.lastMoved);
        REQUIRE(steps <= budget.measured * 4 / 3);
        REQUIRE(steps >= budget.measured * 2 / 3);
    }
}
