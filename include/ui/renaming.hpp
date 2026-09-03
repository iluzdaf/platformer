#pragma once

#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include "game/renames.hpp"
#include "ui/saveable.hpp"

struct Renamed
{
    std::string from, to;
};

using NameTaken = std::function<bool(const std::string &)>;

std::optional<std::string> whyNotARename(
    std::string_view what,
    const std::string &from,
    const std::string &to,
    bool taken);

void rememberRename(Renames &renames, const std::string &from, const std::string &to);

class Renaming
{
public:
    std::optional<Renamed> draw(
        std::string_view what,
        const std::string &selected,
        const NameTaken &taken);

    const Renames &sinceSaved() const;
    void applied(int levelsRewritten);
    void forget();

private:
    std::string typing, lastSelected;
    Renames renames;
    std::optional<int> rewritten;
};

template <class T>
void revertTo(const Saveable &saveable, std::string_view name, T &value, Renaming &renaming)
{
    revertTo(saveable, name, value);
    renaming.forget();
}

struct LevelData;

void writeRenamesIntoLevels(
    Renaming &renaming,
    const std::function<bool(LevelData &, const Renames &)> &rename);
