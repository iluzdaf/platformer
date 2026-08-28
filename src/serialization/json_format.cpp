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
                out += "\n" + std::string(4 * depth, ' ');
        }
        else if (character == '}' || character == ']')
        {
            bool onLines = !expanded.empty() && expanded.back();
            if (!expanded.empty())
                expanded.pop_back();

            --depth;
            if (onLines)
                out += "\n" + std::string(4 * depth, ' ');

            out += character;
        }
        else if (character == ',')
        {
            out += character;
            if (!expanded.empty() && expanded.back())
                out += "\n" + std::string(4 * depth, ' ');
        }
        else
        {
            out += character;
        }
    }

    return out;
}
