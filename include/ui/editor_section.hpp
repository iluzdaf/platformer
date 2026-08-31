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
    Levels,
    Level,
    NpcTypes,
    TilePalettes
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 8> EditorSections{{
    {EditorSection::Playback, "Playback"},
    {EditorSection::Game, "Game"},
    {EditorSection::Camera, "Camera"},
    {EditorSection::Player, "Player"},
    {EditorSection::Levels, "Levels"},
    {EditorSection::Level, "Level"},
    {EditorSection::NpcTypes, "NPC types"},
    {EditorSection::TilePalettes, "Tile palettes"},
}};
