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

struct AnimationWhen : std::map<std::string, Asked>
{
    bool operator==(const AnimationWhen &) const = default;
};

struct BehaviorWhen : std::map<std::string, Asked>
{
    bool operator==(const BehaviorWhen &) const = default;
};
