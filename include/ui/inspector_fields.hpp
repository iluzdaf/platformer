#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
