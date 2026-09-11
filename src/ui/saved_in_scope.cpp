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

    const glz::json_t *walked(const glz::json_t &from)
    {
        const glz::json_t *at = &from;
        for (const std::string &name : path)
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

std::string inspector::pathHere()
{
    std::string dotted;
    for (const std::string &name : path)
        dotted += dotted.empty() ? name : "." + name;

    return dotted;
}

bool inspector::changedFromSaved(const std::string &nowJson)
{
    if (!saved)
        return false;

    const glz::json_t *at = walked(saved.value());
    if (!at)
        return true;

    std::string was;
    if (glz::write_json(*at, was))
        return false;

    return canonical(was) != canonical(nowJson);
}
