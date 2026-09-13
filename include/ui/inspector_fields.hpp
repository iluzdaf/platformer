#pragma once

#include <concepts>
#include <string_view>
#include "ui/inspector_edited.hpp"

struct FrameAnimationData;
struct SheetData;
struct ScoreIconData;
struct HealthIconData;
struct AnimatorData;
struct AnimationWhenData;
struct TransitionWhenData;
struct FactsData;
struct TexturePathData;
struct ScriptPathData;
struct LevelPathData;

inspector::Edited drawCustomField(std::string_view name, FrameAnimationData &value);
inspector::Edited drawCustomField(std::string_view name, SheetData &value);
inspector::Edited drawCustomField(std::string_view name, ScoreIconData &value);
inspector::Edited drawCustomField(std::string_view name, HealthIconData &value);
inspector::Edited drawCustomField(std::string_view name, AnimatorData &value);
inspector::Edited drawCustomField(std::string_view name, AnimationWhenData &value);
inspector::Edited drawCustomField(std::string_view name, TransitionWhenData &value);
inspector::Edited drawCustomField(std::string_view name, FactsData &value);
inspector::Edited drawCustomField(std::string_view name, TexturePathData &value);
inspector::Edited drawCustomField(std::string_view name, ScriptPathData &value);
inspector::Edited drawCustomField(std::string_view name, LevelPathData &value);

namespace inspector
{
    template <class T>
    concept HasCustomField = requires(std::string_view name, T &value) {
        { drawCustomField(name, value) } -> std::same_as<Edited>;
    };
}
