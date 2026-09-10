#pragma once

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <imgui.h>
#include "conditions/asked.hpp"
#include "ui/inspector_edited.hpp"

struct Facts;

struct FactOffered
{
    std::string name;
    AskedKind kind;
};

std::vector<FactOffered> factsOffered(const Facts *declared);

namespace when_field
{
    inspector::Edited drawAsked(std::string_view name, AskedKind kind, Asked &asked);

    bool drawAddingAFact(
        std::map<std::string, Asked> &when,
        std::span<const std::string_view> names,
        std::span<const AskedKind> kinds);
}

template <class Rows>
inspector::Edited drawWhen(
    std::string_view name,
    std::map<std::string, Asked> &when,
    const Rows &rows)
{
    if (!ImGui::TreeNode(std::string(name).c_str()))
        return {};

    inspector::Edited edited;
    std::string takeAway;
    for (const auto &row : rows)
    {
        auto asked = when.find(std::string(row.name));
        if (asked == when.end())
            continue;

        ImGui::PushID(std::string(row.name).c_str());
        if (ImGui::SmallButton("-"))
            takeAway = asked->first;

        ImGui::SameLine();
        edited |= when_field::drawAsked(row.name, row.kind, asked->second);
        ImGui::PopID();
    }

    std::vector<std::string_view> names;
    std::vector<AskedKind> kinds;
    for (const auto &row : rows)
        if (!when.contains(std::string(row.name)))
        {
            names.push_back(row.name);
            kinds.push_back(row.kind);
        }

    bool changed = when_field::drawAddingAFact(when, names, kinds);
    if (!takeAway.empty())
    {
        when.erase(takeAway);
        changed = true;
    }

    if (changed)
        edited |= inspector::Edited{true, true};

    ImGui::TreePop();
    return edited;
}
