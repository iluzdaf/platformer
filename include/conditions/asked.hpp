#pragma once

#include <map>
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

struct AnimationWhenData : std::map<std::string, Asked>
{
    bool operator==(const AnimationWhenData &) const = default;
};

struct BehaviorWhenData : std::map<std::string, Asked>
{
    bool operator==(const BehaviorWhenData &) const = default;
};
