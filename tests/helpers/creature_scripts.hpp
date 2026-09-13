#pragma once

#include <filesystem>
#include <fstream>
#include <string>

inline std::string aScriptThatRuns(const std::string &behavior)
{
    std::string state = std::filesystem::path(behavior).stem().string();
    std::filesystem::path path =
        std::filesystem::temp_directory_path() / ("platformer_runs_" + state + ".lua");
    std::ofstream(path) << "return { states = { " << state << " = include('" << behavior
                        << "') } }\n";
    return path.string();
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
