#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "ui/editor_command.hpp"
#include "ui/editor_commands.hpp"

TEST_CASE("An editor command does nothing when it is asked", "[EditorCommand]")
{
    EditorCommand<> command;
    int handled = 0;
    auto connection = command.connect([&] { ++handled; });

    command();

    REQUIRE(handled == 0);
}

TEST_CASE("An editor command is delivered when the frame drains it", "[EditorCommand]")
{
    EditorCommand<const std::string &> command;
    std::vector<std::string> handled;
    auto connection = command.connect([&](const std::string &path) { handled.push_back(path); });

    command("levels/level2.json");
    command.drain();

    REQUIRE(handled == std::vector<std::string>{"levels/level2.json"});
}

TEST_CASE("An editor command is delivered once", "[EditorCommand]")
{
    EditorCommand<> command;
    int handled = 0;
    auto connection = command.connect([&] { ++handled; });

    command();
    command.drain();
    command.drain();

    REQUIRE(handled == 1);
}

TEST_CASE("An editor command keeps everything asked of it in a frame", "[EditorCommand]")
{
    EditorCommand<const std::string &> command;
    std::vector<std::string> handled;
    auto connection = command.connect([&](const std::string &path) { handled.push_back(path); });

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
    auto connection = command.connect(
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
    auto respawn = commands.onRespawn.connect([&] { handled.emplace_back("respawn"); });
    auto load =
        commands.onLoadLevel.connect([&](const std::string &path) { handled.push_back(path); });

    commands.onRespawn();
    commands.onLoadLevel("levels/level2.json");

    REQUIRE(handled.empty());

    commands.drain();

    REQUIRE(handled == std::vector<std::string>{"respawn", "levels/level2.json"});
}
