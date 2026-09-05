#pragma once

#include <functional>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <map>
#include <utility>
#include <vector>
#include "ui/renames.hpp"
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

std::string nameAfterRenames(const Renames &renames, const std::string &name);

std::string levelsInAList(const std::vector<std::string> &levelPaths);

template <class T>
void renamesTakeEffect(const Renames &renames, std::map<std::string, T> &catalogue)
{
    for (const auto &[was, is] : renames)
    {
        auto node = catalogue.extract(was);
        if (node.empty())
            continue;

        node.key() = is;
        catalogue.insert(std::move(node));
    }
}

class Renaming
{
public:
    std::optional<Renamed> draw(
        std::string_view what,
        const std::string &selected,
        const NameTaken &taken);
    void drawWhatTheLevelsNeed() const;

    void added(const std::string &name);
    bool remove(const std::string &onDisk, const std::optional<std::string> &fallingBackTo);
    bool gone(const std::string &onDisk) const;
    std::vector<std::string> removed() const;

    bool pending() const;
    Renames sinceSaved() const;
    std::string shownName(const std::string &onDisk) const;
    std::string whatTheLevelsNeed() const;
    std::optional<std::string> cannotSaveBecause() const;
    bool somethingIsBecoming(const std::string &name) const;
    void applied();
    void willReach(const std::vector<std::string> &levels);
    void cannotReach(const std::vector<std::string> &levels);
    void forget();

private:
    std::string typing, lastSelected;
    Renames renames;
    std::map<std::string, std::optional<std::string>> removals;
    std::set<std::string> neverSaved;
    std::vector<std::string> willRePoint, unreadable;
};

template <class T>
void revertTo(const Saveable &saveable, std::string_view name, T &value, Renaming &renaming)
{
    revertTo(saveable, name, value);
    renaming.forget();
}

struct LevelData;

bool writeRenamesIntoLevels(
    Renaming &renaming,
    const std::string &directory,
    const std::function<bool(LevelData &, const Renames &)> &rename);

void lookAheadAtLevels(
    Renaming &renaming,
    const std::string &directory,
    const std::function<bool(LevelData &, const Renames &)> &rename);
