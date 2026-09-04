#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include "ui/editor_command.hpp"
#include "ui/editor_commands.hpp"

TEST_CASE("An editor command does nothing when it is asked", "[EditorCommand]")
{
    EditorCommand<> command;
    int handled = 0;
    command.connect([&] { ++handled; });

    command();

    REQUIRE(handled == 0);
}

TEST_CASE("An editor command is delivered when the frame drains it", "[EditorCommand]")
{
    EditorCommand<const std::string &> command;
    std::vector<std::string> handled;
    command.connect([&](const std::string &path) { handled.push_back(path); });

    command("levels/level2.json");
    command.drain();

    REQUIRE(handled == std::vector<std::string>{"levels/level2.json"});
}

TEST_CASE("An editor command is delivered once", "[EditorCommand]")
{
    EditorCommand<> command;
    int handled = 0;
    command.connect([&] { ++handled; });

    command();
    command.drain();
    command.drain();

    REQUIRE(handled == 1);
}

TEST_CASE("An editor command keeps everything asked of it in a frame", "[EditorCommand]")
{
    EditorCommand<const std::string &> command;
    std::vector<std::string> handled;
    command.connect([&](const std::string &path) { handled.push_back(path); });

    command("first");
    command("second");
    command.drain();

    REQUIRE(handled == std::vector<std::string>{"first", "second"});
}

TEST_CASE(
    "An editor command asked for again while draining waits for the next drain",
    "[EditorCommand]")
{
    EditorCommand<> command;
    int handled = 0;
    command.connect(
        [&]
        {
            ++handled;
            if (handled < 5)
                command();
        });

    command();
    command.drain();

    REQUIRE(handled == 1);

    command.drain();

    REQUIRE(handled == 2);
}

TEST_CASE("Draining the editor commands delivers all of them", "[EditorCommand]")
{
    EditorCommands commands;
    std::vector<std::string> handled;
    commands.onRespawn.connect([&] { handled.emplace_back("respawn"); });
    commands.onLoadLevel.connect([&](const std::string &path) { handled.push_back(path); });

    commands.onRespawn();
    commands.onLoadLevel("levels/level2.json");

    REQUIRE(handled.empty());

    commands.drain();

    REQUIRE(handled == std::vector<std::string>{"respawn", "levels/level2.json"});
}

TEST_CASE("An editor command stays connected when the caller keeps nothing", "[EditorCommand]")
{
    EditorCommands commands;
    int respawns = 0;
    std::vector<std::string> loaded;

    commands.onRespawn.connect([&respawns] { ++respawns; });
    commands.onLoadLevel.connect([&loaded](const std::string &path) { loaded.push_back(path); });

    commands.onRespawn();
    commands.onLoadLevel("levels/level2.json");
    commands.drain();

    REQUIRE(respawns == 1);
    REQUIRE(loaded == std::vector<std::string>{"levels/level2.json"});
}

TEST_CASE(
    "An editor command whose handler throws is reported rather than escaping",
    "[EditorCommand]")
{
    EditorCommand<> command;
    command.connect([] { throw std::runtime_error("cannot build the level"); });

    command();

    REQUIRE_NOTHROW(command.drain());
}

TEST_CASE("One editor command that throws does not stop the next", "[EditorCommand]")
{
    EditorCommand<int> command;
    std::vector<int> delivered;
    command.connect(
        [&](int asked)
        {
            if (asked == 1)
                throw std::runtime_error("cannot build the level");

            delivered.push_back(asked);
        });

    command(1);
    command(2);
    REQUIRE_NOTHROW(command.drain());

    REQUIRE(delivered == std::vector<int>{2});
}
