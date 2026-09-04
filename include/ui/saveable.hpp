#pragma once

#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <glaze/glaze.hpp>
#include <utility>

class Saveable
{
public:
    void seen(std::string_view name, const std::string &now)
    {
        std::string &lastSeen = asLastSeen[std::string(name)];
        if (lastSeen.empty())
            lastSeen = now;
    }

    bool unsavedSince(std::string_view name, const std::string &now)
    {
        seen(name, now);

        return unsaved(name, now);
    }

    bool unsaved(std::string_view name, const std::string &now) const
    {
        auto it = asLastSeen.find(std::string(name));
        return it != asLastSeen.end() && !it->second.empty() && it->second != now;
    }

    void saved(std::string_view name, std::string now)
    {
        asLastSeen[std::string(name)] = std::move(now);
    }

    std::string lastSeen(std::string_view name) const
    {
        auto it = asLastSeen.find(std::string(name));
        return it == asLastSeen.end() ? std::string() : it->second;
    }

    void valuesReplaced()
    {
        asLastSeen.clear();
    }

private:
    std::map<std::string, std::string> asLastSeen;
};

template <class T> void revertTo(const Saveable &saveable, std::string_view name, T &value)
{
    T asItWas;
    if (glz::read_json(asItWas, saveable.lastSeen(name)))
    {
        std::cerr << "could not put " << name << " back\n";
        return;
    }

    value = std::move(asItWas);
}

template <class T> std::string asJson(const T &value)
{
    std::string json;
    if (glz::write_json(value, json))
        throw std::runtime_error("Failed to serialise for comparison");

    return json;
}

template <class T>
void reload(Saveable &saveable, std::string_view name, T &current, const T &onDisk)
{
    if (!saveable.unsaved(name, asJson(current)))
        current = onDisk;

    saveable.saved(name, asJson(onDisk));
}
