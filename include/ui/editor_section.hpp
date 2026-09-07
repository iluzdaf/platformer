#pragma once

#include <array>
#include <string_view>
#include <utility>

enum class EditorSection
{
    Runtime,
    Game,
    Cast,
    Level
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 4> EditorSections{{
    {EditorSection::Runtime, "Runtime"},
    {EditorSection::Game, "Game"},
    {EditorSection::Cast, "Cast"},
    {EditorSection::Level, "Level"},
}};
