#pragma once

#include <string>
#include <utility>
#include <glaze/glaze.hpp>

struct ScriptPathData
{
    std::string path;

    ScriptPathData() = default;
    ScriptPathData(std::string path) : path(std::move(path))
    {
    }
    ScriptPathData(const char *path) : path(path)
    {
    }

    bool operator==(const ScriptPathData &) const = default;
};

template <> struct glz::meta<ScriptPathData>
{
    // NOLINTNEXTLINE(readability-identifier-naming) glaze requires this name
    static constexpr auto value = &ScriptPathData::path;
};
