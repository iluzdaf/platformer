#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"

struct FrameAnimationData;
struct SheetData;
struct ScoreIconData;
struct ActorAnimationData;
struct AnimationWhen;
struct BehaviorWhen;

inspector::Edited drawCustomField(std::string_view name, FrameAnimationData &value);
inspector::Edited drawCustomField(std::string_view name, SheetData &value);
inspector::Edited drawCustomField(std::string_view name, ScoreIconData &value);
inspector::Edited drawCustomField(std::string_view name, ActorAnimationData &value);
inspector::Edited drawCustomField(std::string_view name, AnimationWhen &value);
inspector::Edited drawCustomField(std::string_view name, BehaviorWhen &value);

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
