#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>
#include "serialization/json_format.hpp"

namespace
{
    constexpr int NestingOnLines = 2;
    constexpr size_t InlineWidthLimit = 100;
}

struct Span
{
    bool holdsContainer = false;
    size_t length = 0;
};

Span spanOf(const std::string &json, size_t opening)
{
    Span span;
    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (size_t at = opening; at < json.size(); ++at)
    {
        char character = json[at];

        if (escaped)
            escaped = false;
        else if (inString)
        {
            if (character == '\\')
                escaped = true;
            else if (character == '"')
                inString = false;
        }
        else if (character == '"')
            inString = true;
        else if (character == '{' || character == '[')
        {
            if (++depth > 1)
                span.holdsContainer = true;
        }
        else if (character == '}' || character == ']')
        {
            if (--depth == 0)
            {
                span.length = at - opening + 1;
                return span;
            }
        }
    }

    return span;
}

std::string withPaddedGrid(const std::string &json)
{
    const std::string key = "\"indices\":[";
    size_t start = json.find(key);
    if (start == std::string::npos)
        return json;

    size_t cursor = start + key.size();
    std::vector<std::vector<std::string>> rows;

    while (cursor < json.size() && json[cursor] == '[')
    {
        size_t end = json.find(']', cursor);
        if (end == std::string::npos)
            return json;

        std::vector<std::string> cells;
        for (size_t cell = cursor + 1; cell < end;)
        {
            size_t comma = json.find(',', cell);
            if (comma == std::string::npos || comma > end)
                comma = end;

            cells.push_back(json.substr(cell, comma - cell));
            cell = comma + 1;
        }
        rows.push_back(std::move(cells));

        cursor = end + 1;
        if (cursor < json.size() && json[cursor] == ',')
            ++cursor;
    }

    size_t width = 0;
    for (const auto &row : rows)
        for (const auto &cell : row)
            width = std::max(width, cell.size());

    std::string out = json.substr(0, start + key.size());
    for (size_t row = 0; row < rows.size(); ++row)
    {
        out += "[";
        for (size_t cell = 0; cell < rows[row].size(); ++cell)
        {
            if (cell > 0)
                out += ",";

            out += std::string(width - rows[row][cell].size(), ' ') + rows[row][cell];
        }
        out += "]";

        if (row + 1 < rows.size())
            out += ",";
    }

    return out + json.substr(cursor);
}

std::string withStructureOnLines(const std::string &json)
{
    std::string out;
    std::vector<bool> expanded;
    int depth = 0;
    bool inString = false;
    bool escaped = false;

    for (size_t at = 0; at < json.size(); ++at)
    {
        char character = json[at];

        if (escaped)
        {
            out += character;
            escaped = false;
            continue;
        }

        if (inString)
        {
            out += character;
            if (character == '\\')
                escaped = true;
            else if (character == '"')
                inString = false;
            continue;
        }

        if (character == '"')
        {
            out += character;
            inString = true;
        }
        else if (character == '{' || character == '[')
        {
            char closing = character == '{' ? '}' : ']';
            if (at + 1 < json.size() && json[at + 1] == closing)
            {
                out += character;
                out += closing;
                ++at;
                continue;
            }

            size_t lineStart = out.rfind('\n');
            size_t column =
                lineStart == std::string::npos ? out.size() : out.size() - lineStart - 1;

            Span span = spanOf(json, at);
            bool nearTop = span.holdsContainer && depth + 1 <= NestingOnLines;
            bool tooLong = column + span.length > InlineWidthLimit;
            bool scalarArray = character == '[' && !span.holdsContainer;

            bool onLines = !scalarArray && (nearTop || tooLong);
            out += character;
            ++depth;
            expanded.push_back(onLines);

            if (onLines)
                out += "\n" + std::string(static_cast<std::size_t>(4 * depth), ' ');
        }
        else if (character == '}' || character == ']')
        {
            bool onLines = !expanded.empty() && expanded.back();
            if (!expanded.empty())
                expanded.pop_back();

            --depth;
            if (onLines)
                out += "\n" + std::string(static_cast<std::size_t>(4 * depth), ' ');

            out += character;
        }
        else if (character == ',')
        {
            out += character;
            if (!expanded.empty() && expanded.back())
                out += "\n" + std::string(static_cast<std::size_t>(4 * depth), ' ');
        }
        else
        {
            out += character;
        }
    }

    return out;
}
