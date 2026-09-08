#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <utility>
#include <glaze/glaze.hpp>
#include "assets/sheet_data.hpp"
#include "serialization/data_shapes.hpp"

namespace fitting
{
    template <class Field>
    const SheetData *orTheOneAlreadyFound(const SheetData *found, const Field &field)
    {
        if constexpr (std::is_same_v<std::remove_cvref_t<Field>, SheetData>)
            return found ? found : &field;
        else
            return found;
    }
}

template <class T> consteval bool holdsASheet()
{
    if constexpr (!glz::reflectable<T>)
        return false;
    else
        return []<std::size_t... I>(std::index_sequence<I...>)
        {
            return (
                std::is_same_v<
                    std::remove_cvref_t<typename glz::reflect<T>::template type<I>>,
                    SheetData> ||
                ...);
        }(std::make_index_sequence<glz::reflect<T>::size>{});
}

template <class T> const SheetData &theSheetIn(const T &value)
{
    static_assert(holdsASheet<T>(), "This data holds no sheet, so there is none to hand back");

    const SheetData *found = nullptr;
    [&]<std::size_t... I>(std::index_sequence<I...>)
    {
        ((found = fitting::orTheOneAlreadyFound(found, glz::get<I>(glz::to_tie(value)))), ...);
    }(std::make_index_sequence<glz::reflect<T>::size>{});

    return *found;
}

template <class T>
concept SaysWhatMustFit =
    requires(const T &value, const std::string &whose) { checkFits(value, whose, 0, 0); };

template <class T, class Check>
void everySheetFittingIn(const T &value, const std::string &whose, Check &&check)
{
    if constexpr (shapes::IsOptional<T>::value)
    {
        if (value)
            everySheetFittingIn(*value, whose, check);
    }
    else if constexpr (shapes::IsVector<T>::value)
    {
        std::size_t at = 0;
        for (const auto &held : value)
            everySheetFittingIn(held, whose + "[" + std::to_string(at++) + "]", check);
    }
    else if constexpr (shapes::IsMap<T>::value)
    {
        for (const auto &[key, held] : value)
            everySheetFittingIn(held, whose + " \"" + shapes::keyText(key) + "\"", check);
    }
    else if constexpr (glz::reflectable<T>)
    {
        if constexpr (holdsASheet<T>())
        {
            static_assert(
                SaysWhatMustFit<T>,
                "This data holds a sheet, so it must say what has to fit in it. Give it a "
                "checkFits(const T &, const std::string &whose, int width, int height).");

            check(value, whose);
        }
        else
            [&]<std::size_t... I>(std::index_sequence<I...>)
            {
                (everySheetFittingIn(
                     glz::get<I>(glz::to_tie(value)),
                     whose.empty() ? std::string(glz::reflect<T>::keys[I])
                                   : whose + "." + std::string(glz::reflect<T>::keys[I]),
                     check),
                 ...);
            }(std::make_index_sequence<glz::reflect<T>::size>{});
    }
}
