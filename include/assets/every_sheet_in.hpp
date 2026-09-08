#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <glaze/glaze.hpp>
#include "assets/sheet_data.hpp"
#include "serialization/data_shapes.hpp"

template <class T, class Visit> void everySheetIn(const T &value, Visit &&visit)
{
    if constexpr (std::is_same_v<T, SheetData>)
        visit(value);
    else if constexpr (shapes::IsOptional<T>::value)
    {
        if (value)
            everySheetIn(*value, visit);
    }
    else if constexpr (shapes::IsVector<T>::value)
    {
        for (const auto &held : value)
            everySheetIn(held, visit);
    }
    else if constexpr (shapes::IsMap<T>::value)
    {
        for (const auto &[key, held] : value)
            everySheetIn(held, visit);
    }
    else if constexpr (glz::reflectable<T>)
    {
        constexpr auto Fields = glz::reflect<T>::size;
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (everySheetIn(glz::get<I>(glz::to_tie(value)), visit), ...);
        }(std::make_index_sequence<Fields>{});
    }
}
