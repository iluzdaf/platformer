#pragma once

#include <string>
#include <utility>
#include <glaze/glaze.hpp>

struct LevelPathData
{
    std::string path;

    LevelPathData() = default;
    LevelPathData(std::string path) : path(std::move(path))
    {
    }
    LevelPathData(const char *path) : path(path)
    {
    }

    bool operator==(const LevelPathData &) const = default;
};

template <> struct glz::meta<LevelPathData>
{
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = &LevelPathData::path;
};
