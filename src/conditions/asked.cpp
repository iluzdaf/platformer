#include <format>
#include <string>
#include <string_view>
#include "conditions/asked.hpp"

AskedKind kindOf(const Asked &asked)
{
    switch (asked.index())
    {
    case 0:
        return AskedKind::YesOrNo;
    case 1:
        return AskedKind::Number;
    default:
        return AskedKind::Name;
    }
}

std::string_view nameOf(AskedKind kind)
{
    switch (kind)
    {
    case AskedKind::YesOrNo:
        return "a yes or no";
    case AskedKind::Number:
        return "a number";
    case AskedKind::Name:
        return "a name";
    }

    return "";
}

Asked emptyOf(AskedKind kind)
{
    switch (kind)
    {
    case AskedKind::YesOrNo:
        return false;
    case AskedKind::Number:
        return 0.0f;
    case AskedKind::Name:
        return std::string();
    }

    return false;
}

std::string textOf(const Asked &asked)
{
    switch (kindOf(asked))
    {
    case AskedKind::YesOrNo:
        return std::get<bool>(asked) ? "yes" : "no";
    case AskedKind::Number:
        return std::format("{}", std::get<float>(asked));
    case AskedKind::Name:
        return std::format("\"{}\"", std::get<std::string>(asked));
    }

    return "";
}
