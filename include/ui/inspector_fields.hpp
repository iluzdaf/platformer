#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"

struct FrameAnimationData;
struct TileColliderData;

inspector::Edited drawCustomField(std::string_view name, FrameAnimationData &value);
inspector::Edited drawCustomField(std::string_view name, TileColliderData &value);

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
