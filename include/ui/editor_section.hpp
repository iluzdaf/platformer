#pragma once

#include <array>
#include <string_view>
#include <utility>

enum class EditorSection
{
    Runtime,
    Game,
    Player,
    Level,
    Types
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 5> EditorSections{{
    {EditorSection::Runtime, "Runtime"},
    {EditorSection::Game, "Game"},
    {EditorSection::Player, "Player"},
    {EditorSection::Level, "Level"},
    {EditorSection::Types, "Types"},
}};
