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
    TileMap,
    NpcsInLevel,
    NpcTypes,
    TilePalettes,
    Navigation
};

inline constexpr std::array<std::pair<EditorSection, std::string_view>, 10> EditorSections{{
    {EditorSection::Playback, "Playback"},
    {EditorSection::Game, "Game"},
    {EditorSection::Camera, "Camera"},
    {EditorSection::Player, "Player"},
    {EditorSection::Level, "Level"},
    {EditorSection::TileMap, "Tile map"},
    {EditorSection::NpcsInLevel, "NPCs in level"},
    {EditorSection::NpcTypes, "NPC types"},
    {EditorSection::TilePalettes, "Tile palettes"},
    {EditorSection::Navigation, "Navigation"},
}};
