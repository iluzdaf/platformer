#pragma once

#include <array>
#include <string_view>

enum class EditorSection
{
    Playback,
    Camera,
    Player,
    Level,
    Npcs,
    TilePalette,
    Navigation
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 7> EditorSections{{
    {EditorSection::Playback, "Playback"},
    {EditorSection::Camera, "Camera"},
    {EditorSection::Player, "Player"},
    {EditorSection::Level, "Level"},
    {EditorSection::Npcs, "NPCs"},
    {EditorSection::TilePalette, "Tile palette"},
    {EditorSection::Navigation, "Navigation"},
}};
