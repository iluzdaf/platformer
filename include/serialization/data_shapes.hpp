#pragma once

#include <map>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

namespace shapes
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

    template <class K> std::string keyText(const K &key)
    {
        if constexpr (std::is_arithmetic_v<K>)
            return std::to_string(key);
        else
            return std::string(key);
    }
}
