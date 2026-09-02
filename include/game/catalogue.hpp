#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <string_view>

template <class T>
const T &oneNamed(
    const std::map<std::string, T> &catalogue,
    std::string_view kind,
    const std::string &name)
{
    auto known = catalogue.find(name);
    if (known == catalogue.end())
        throw std::runtime_error("No " + std::string(kind) + " is named \"" + name + "\"");

    return known->second;
}
