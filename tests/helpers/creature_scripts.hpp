#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

inline std::string aScriptThatRuns(const std::vector<std::string> &behaviors)
{
    std::string name = "platformer_runs";
    std::string states;
    for (const std::string &behavior : behaviors)
    {
        std::string state = std::filesystem::path(behavior).stem().string();
        name += "_" + state;
        states += state + " = include('" + behavior + "'), ";
    }

    std::filesystem::path path = std::filesystem::temp_directory_path() / (name + ".lua");
    std::ofstream(path) << "return { states = { " << states << "} }\n";
    return path.string();
}

inline std::string aScriptThatRuns(const std::string &behavior)
{
    return aScriptThatRuns(std::vector<std::string>{behavior});
}

inline std::string aScriptThatWalksRight()
{
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / "platformer_walks_right.lua";
    std::ofstream(path) << "return { states = { walk = { decide = function()\n"
                           "    local wants = Intentions.new()\n"
                           "    wants.direction = vec2.new(1, 0)\n"
                           "    return wants\n"
                           "end } } }\n";
    return path.string();
}
