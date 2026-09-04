#include <catch2/catch_test_macros.hpp>
#include <efsw/efsw.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include "assets/asset_paths.hpp"
#include "reloading/file_watcher_listener.hpp"

namespace
{
    struct Listening
    {
        FileWatcherListener listener;
        std::vector<std::string> delivered;

        Listening()
        {
            listener.onFileModified = [this](const std::string &path)
            { delivered.push_back(path); };
        }

        void seen(const std::string &directory, const std::string &file, efsw::Action action)
        {
            listener.handleFileAction(0, directory, file, action, "");
        }
    };
}

TEST_CASE("A modified file is delivered under the asset root once processed", "[Reloading]")
{
    Listening listening;

    listening.seen(assets::root(), "camera.json", efsw::Actions::Modified);
    REQUIRE(listening.delivered.empty());

    listening.listener.process();

    REQUIRE(listening.delivered == std::vector<std::string>{"camera.json"});
}

TEST_CASE("A file added or moved in counts as modified", "[Reloading]")
{
    Listening listening;

    listening.seen(assets::root(), "player.json", efsw::Actions::Add);
    listening.seen(assets::root(), "npcs.json", efsw::Actions::Moved);
    listening.listener.process();

    REQUIRE(listening.delivered == std::vector<std::string>{"player.json", "npcs.json"});
}

TEST_CASE("A deleted file is not delivered", "[Reloading]")
{
    Listening listening;

    listening.seen(assets::root(), "camera.json", efsw::Actions::Delete);
    listening.listener.process();

    REQUIRE(listening.delivered.empty());
}

TEST_CASE("Each change is delivered once", "[Reloading]")
{
    Listening listening;
    listening.seen(assets::root(), "camera.json", efsw::Actions::Modified);
    listening.listener.process();

    listening.listener.process();

    REQUIRE(listening.delivered.size() == 1);
}

TEST_CASE("A file in a subdirectory keeps its path under the root", "[Reloading]")
{
    Listening listening;
    std::string levels = (std::filesystem::path(assets::root()) / "levels").string();

    listening.seen(levels, "level1.json", efsw::Actions::Modified);
    listening.listener.process();

    REQUIRE(listening.delivered == std::vector<std::string>{"levels/level1.json"});
}
