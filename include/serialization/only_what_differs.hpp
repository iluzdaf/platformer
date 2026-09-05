#pragma once

#include <cstddef>
#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <glaze/glaze.hpp>

namespace differs
{
    template <class T> struct IsOptional : std::false_type
    {
    };
    template <class T> struct IsOptional<std::optional<T>> : std::true_type
    {
    };

    template <class T> struct IsVector : std::false_type
    {
    };
    template <class T> struct IsVector<std::vector<T>> : std::true_type
    {
    };

    template <class T> struct IsMap : std::false_type
    {
    };
    template <class K, class V> struct IsMap<std::map<K, V>> : std::true_type
    {
    };

    template <class T> std::string compact(const T &value)
    {
        std::string json;
        if (glz::write_json(value, json))
            throw std::runtime_error("Failed to serialise for the file");

        return json;
    }

    template <class K> std::string keyText(const K &key)
    {
        if constexpr (std::is_arithmetic_v<K>)
            return std::to_string(key);
        else
            return std::string(key);
    }

    template <class T> std::string onlyWhatDiffers(const T &value, const T &fromDefault);

    template <class T, class Visit>
    void forEachField(const T &value, const T &fromDefault, Visit &&visit)
    {
        constexpr auto Fields = glz::reflect<T>::size;
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (visit(
                 glz::reflect<T>::keys[I],
                 glz::get<I>(glz::to_tie(value)),
                 glz::get<I>(glz::to_tie(fromDefault))),
             ...);
        }(std::make_index_sequence<Fields>{});
    }

    template <class T> std::string onlyWhatDiffers(const T &value, const T &fromDefault)
    {
        if constexpr (IsOptional<T>::value)
        {
            if (!value)
                return "null";

            using Held = typename T::value_type;
            return onlyWhatDiffers(*value, fromDefault ? *fromDefault : Held{});
        }
        else if constexpr (IsVector<T>::value)
        {
            using Held = typename T::value_type;
            std::string out = "[";
            for (std::size_t at = 0; at < value.size(); ++at)
                out += (at > 0 ? "," : "") + onlyWhatDiffers(value[at], Held{});

            return out + "]";
        }
        else if constexpr (IsMap<T>::value)
        {
            using Held = typename T::mapped_type;
            std::string out = "{";
            bool first = true;
            for (const auto &[key, held] : value)
            {
                out += (first ? "" : ",") + compact(keyText(key)) + ":" +
                       onlyWhatDiffers(held, Held{});
                first = false;
            }

            return out + "}";
        }
        else if constexpr (glz::reflectable<T>)
        {
            std::string out = "{";
            bool first = true;
            forEachField(
                value,
                fromDefault,
                [&](std::string_view name, const auto &field, const auto &fieldDefault)
                {
                    if (compact(field) == compact(fieldDefault))
                        return;

                    out += (first ? "" : ",") + compact(std::string(name)) + ":" +
                           onlyWhatDiffers(field, fieldDefault);
                    first = false;
                });

            return out + "}";
        }
        else
            return compact(value);
    }
}

template <class T> std::string onlyWhatDiffers(const T &value)
{
    return differs::onlyWhatDiffers(value, T{});
}
