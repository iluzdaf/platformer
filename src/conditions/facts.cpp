#include <optional>
#include <string>
#include "conditions/facts.hpp"
#include "conditions/asked.hpp"

std::optional<std::string> whyNotDeclared(
    const Facts &declared,
    const std::string &name,
    const Asked &value)
{
    auto fact = declared.find(name);
    if (fact == declared.end())
        return "\"" + name + "\" is not a declared fact";

    if (kindOf(fact->second) != kindOf(value))
        return "\"" + name + "\" is " + std::string(nameOf(kindOf(fact->second))) +
               ", and was given " + std::string(nameOf(kindOf(value)));

    return std::nullopt;
}
