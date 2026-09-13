#pragma once

#include <string>
#include <string_view>
#include <variant>

using Asked = std::variant<bool, float, std::string>;

enum class AskedKind
{
    YesOrNo,
    Number,
    Name
};

AskedKind kindOf(const Asked &asked);

std::string_view nameOf(AskedKind kind);

Asked emptyOf(AskedKind kind);

std::string textOf(const Asked &asked);
