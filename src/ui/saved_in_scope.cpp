#include <charconv>
#include <cerrno>
#include <system_error>
#include <utility>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
#include <glaze/glaze.hpp>
#include "ui/saved_in_scope.hpp"

namespace
{
    std::optional<glz::json_t> saved;
    std::vector<std::string> path;
    std::vector<std::string> pathsLooked;
    std::vector<std::string> pathsChanged;
    bool watching = false;

    std::string canonical(const std::string &json)
    {
        glz::json_t read;
        if (glz::read_json(read, json))
            return json;

        std::string written;
        if (glz::write_json(read, written))
            return json;

        return written;
    }

    const glz::json_t *walked(const glz::json_t &from, const std::vector<std::string> &names)
    {
        const glz::json_t *at = &from;
        for (const std::string &name : names)
        {
            if (at->holds<glz::json_t::object_t>())
            {
                const auto &held = at->get<glz::json_t::object_t>();
                auto found = held.find(name);
                if (found == held.end())
                    return nullptr;

                at = &found->second;
                continue;
            }

            if (at->holds<glz::json_t::array_t>())
            {
                const auto &held = at->get<glz::json_t::array_t>();
                std::size_t index = 0;
                auto [end, error] = std::from_chars(name.data(), name.data() + name.size(), index);
                if (error != std::errc{} || index >= held.size())
                    return nullptr;

                at = &held[index];
                continue;
            }

            return nullptr;
        }

        return at;
    }
}

SavedInScope::SavedInScope(const std::string &savedJson)
{
    glz::json_t read;
    if (!glz::read_json(read, savedJson))
        saved = std::move(read);

    path.clear();
}

SavedInScope::~SavedInScope()
{
    saved.reset();
    path.clear();
}

inspector::InField::InField(std::string_view name)
{
    path.emplace_back(name);
}

inspector::InField::~InField()
{
    path.pop_back();
}

inspector::Watching::Watching()
{
    pathsLooked.clear();
    pathsChanged.clear();
    watching = true;
}

inspector::Watching::~Watching()
{
    watching = false;
    pathsLooked.clear();
    pathsChanged.clear();
}

std::vector<std::string> inspector::Watching::looked() const
{
    return pathsLooked;
}

std::vector<std::string> inspector::Watching::saidChanged() const
{
    return pathsChanged;
}

std::string inspector::pathHere()
{
    std::string dotted;
    for (const std::string &name : path)
        dotted += dotted.empty() ? name : "." + name;

    return dotted;
}

namespace
{
    bool askedOfWhatWasSaved(
        const glz::json_t &from,
        const std::vector<std::string> &names,
        const std::string &nowJson)
    {
        const glz::json_t *at = walked(from, names);
        if (!at)
            return nowJson != "null";

        std::string was;
        if (glz::write_json(*at, was))
            return false;

        return canonical(was) != canonical(nowJson);
    }
}

bool inspector::changedAt(const std::vector<std::string> &at, const std::string &nowJson)
{
    if (!saved)
        return false;

    return askedOfWhatWasSaved(*saved, at, nowJson);
}

bool inspector::changedFromSaved(const std::string &nowJson)
{
    if (!saved)
        return false;

    bool answer = askedOfWhatWasSaved(*saved, path, nowJson);
    if (watching)
    {
        pathsLooked.push_back(pathHere());
        if (answer)
            pathsChanged.push_back(pathHere());
    }

    return answer;
}
