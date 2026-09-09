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
