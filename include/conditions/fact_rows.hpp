#pragma once

#include <cstddef>
#include <format>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include "conditions/asked.hpp"

template <class Context> struct FactRow
{
    std::string_view name;
    AskedKind kind;
    const char *yes;
    const char *no;
    bool (*holds)(const Asked &asked, const Context &context);
};

template <class Context>
const FactRow<Context> *rowNamed(std::span<const FactRow<Context>> rows, std::string_view name)
{
    for (const FactRow<Context> &row : rows)
        if (row.name == name)
            return &row;

    return nullptr;
}

template <class Context>
std::optional<std::string> whyNotAsked(
    const std::map<std::string, Asked> &when,
    std::span<const FactRow<Context>> rows)
{
    for (const auto &[name, asked] : when)
    {
        const FactRow<Context> *row = rowNamed(rows, name);
        if (!row)
            return "asks about \"" + name + "\", and there is no such fact";

        if (row->kind != kindOf(asked))
            return "asks \"" + name + "\" with " + std::string(nameOf(kindOf(asked))) +
                   ", and it wants " + std::string(nameOf(row->kind));
    }

    return std::nullopt;
}

template <class Context>
bool holds(
    const std::map<std::string, Asked> &when,
    std::span<const FactRow<Context>> rows,
    const Context &context)
{
    for (const auto &[name, asked] : when)
    {
        const FactRow<Context> *row = rowNamed(rows, name);
        if (!row)
            throw std::runtime_error(
                "A condition asks about \"" + name + "\", and there is no such fact");

        if (!row->holds(asked, context))
            return false;
    }

    return true;
}

template <class Context> std::string wordsOf(const FactRow<Context> &row, const Asked &asked)
{
    switch (kindOf(asked))
    {
    case AskedKind::YesOrNo:
        return std::get<bool>(asked) ? row.yes : row.no;
    case AskedKind::Number:
        return std::format("{} {}", row.yes, std::get<float>(asked));
    case AskedKind::Name:
        return std::format("{} \"{}\"", row.yes, std::get<std::string>(asked));
    }

    return "";
}

template <class Context>
std::string whenOf(const std::map<std::string, Asked> &when, std::span<const FactRow<Context>> rows)
{
    std::string text;
    for (const FactRow<Context> &row : rows)
    {
        auto asked = when.find(std::string(row.name));
        if (asked == when.end())
            continue;

        text += (text.empty() ? "" : ", ") + wordsOf(row, asked->second);
    }

    return text.empty() ? "always" : text;
}
