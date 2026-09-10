#pragma once

#include <string>
#include <utility>
#include <glaze/glaze.hpp>

struct TexturePathData
{
    std::string path;

    TexturePathData() = default;
    TexturePathData(std::string path) : path(std::move(path))
    {
    }
    TexturePathData(const char *path) : path(path)
    {
    }

    bool operator==(const TexturePathData &) const = default;
};

template <> struct glz::meta<TexturePathData>
{
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = &TexturePathData::path;
};
