#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <type_traits>
#include <vector>
#include <glaze/glaze.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "assets/texture_path_data.hpp"
#include "game/level_path_data.hpp"
#include "scripting/script_path_data.hpp"
#include "serialization/data_shapes.hpp"

namespace leaves
{
    template <class T>
    constexpr bool NamesAPath =
        std::is_same_v<T, TexturePathData> || std::is_same_v<T, ScriptPathData> ||
        std::is_same_v<T, LevelPathData>;

    template <class T> std::vector<T> insteadOf(const T &value)
    {
        if constexpr (std::is_same_v<T, bool>)
            return {!value};
        else if constexpr (std::is_same_v<T, int>)
            return {0, -1};
        else if constexpr (std::is_same_v<T, float>)
            return {0.0f, -1.0f};
        else if constexpr (std::is_same_v<T, std::string>)
            return {std::string(), "nowhere"};
        else if constexpr (std::is_same_v<T, glm::vec2>)
            return {glm::vec2(0.0f), glm::vec2(-1.0f)};
        else if constexpr (std::is_same_v<T, glm::ivec2>)
            return {glm::ivec2(0), glm::ivec2(-1)};
        else if constexpr (NamesAPath<T>)
            return {T(std::string()), T("nowhere")};
        else
            return {};
    }

    template <class T>
    constexpr bool Suspectable =
        std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, float> ||
        std::is_same_v<T, std::string> || std::is_same_v<T, glm::vec2> ||
        std::is_same_v<T, glm::ivec2> || NamesAPath<T>;

    template <class T> std::vector<T> suspicious(const T &value)
    {
        if constexpr (!Suspectable<T>)
            return {};
        else
        {
            std::vector<T> bad = insteadOf(value);
            std::erase(bad, value);

            return bad;
        }
    }

    template <class T, class Visit> void eachLeafOf(T &value, const std::string &at, Visit &&visit);

    template <class T, class Visit>
    void eachLeafUnder(const std::string &name, T &value, const std::string &at, Visit &&visit)
    {
        std::string here = at.empty() ? name : at + "." + name;
        if (!suspicious(value).empty())
            visit(here, value);

        eachLeafOf(value, here, visit);
    }

    template <class T, class Visit> void eachLeafOf(T &value, const std::string &at, Visit &&visit)
    {
        if constexpr (shapes::IsOptional<T>::value)
        {
            if (value)
                eachLeafOf(*value, at, visit);
        }
        else if constexpr (shapes::IsVector<T>::value)
        {
            for (std::size_t index = 0; index < value.size(); ++index)
                eachLeafUnder(std::to_string(index), value[index], at, visit);
        }
        else if constexpr (shapes::IsMap<T>::value)
        {
            for (auto &[key, held] : value)
                eachLeafUnder(shapes::keyText(key), held, at, visit);
        }
        else if constexpr (shapes::IsVariant<T>::value && glz::tagged<T>)
        {
            std::visit(
                [&at, &visit](auto &held)
                {
                    constexpr auto Fields = glz::reflect<std::remove_cvref_t<decltype(held)>>::size;
                    [&]<std::size_t... I>(std::index_sequence<I...>)
                    {
                        (eachLeafUnder(
                             std::string(
                                 glz::reflect<std::remove_cvref_t<decltype(held)>>::keys[I]),
                             glz::get<I>(glz::to_tie(held)),
                             at,
                             visit),
                         ...);
                    }(std::make_index_sequence<Fields>{});
                },
                value);
        }
        else if constexpr (glz::reflectable<T> && !NamesAPath<T>)
        {
            constexpr auto Fields = glz::reflect<T>::size;
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                (eachLeafUnder(
                     std::string(glz::reflect<T>::keys[I]),
                     glz::get<I>(glz::to_tie(value)),
                     at,
                     visit),
                 ...);
            }(std::make_index_sequence<Fields>{});
        }
    }

    template <class T, class Visit> void eachLeafOf(T &value, Visit &&visit)
    {
        eachLeafOf(value, std::string(), visit);
    }
}
