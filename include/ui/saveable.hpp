#pragma once

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <glaze/glaze.hpp>
#include "serialization/only_what_differs.hpp"
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
    return onlyWhatDiffers(value);
}

template <class T>
bool reload(Saveable &saveable, std::string_view name, T &current, const T &onDisk)
{
    std::string held = asJson(current);
    std::string fromDisk = asJson(onDisk);
    bool taken = !saveable.unsaved(name, held) && held != fromDisk;
    if (!saveable.unsaved(name, held))
        current = onDisk;

    saveable.saved(name, fromDisk);
    return taken;
}
