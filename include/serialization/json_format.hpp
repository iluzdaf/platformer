#pragma once

#include <string>
#include "serialization/only_what_differs.hpp"

std::string withPaddedGrid(const std::string &json);
std::string withStructureOnLines(const std::string &json);

template <class T> std::string asFileText(const T &value)
{
    return withStructureOnLines(withPaddedGrid(onlyWhatDiffers(value)));
}
