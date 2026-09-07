#pragma once

#include <array>
#include <string_view>
#include <utility>

enum class EditorSection
{
    Playback,
    Game,
    Camera,
    Player,
    Level,
    Types
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 6> EditorSections{{
    {EditorSection::Playback, "Playback"},
    {EditorSection::Game, "Game"},
    {EditorSection::Camera, "Camera"},
    {EditorSection::Player, "Player"},
    {EditorSection::Level, "Level"},
    {EditorSection::Types, "Types"},
}};
