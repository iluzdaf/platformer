#pragma once

#include <array>
#include <string_view>
#include <utility>

enum class EditorSection
{
    Runtime,
    Game,
    Level
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 3> EditorSections{{
    {EditorSection::Runtime, "Runtime"},
    {EditorSection::Game, "Game"},
    {EditorSection::Level, "Level"},
}};
