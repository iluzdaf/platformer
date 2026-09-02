#pragma once

#include <algorithm>
#include <array>
#include <cfloat>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <glaze/glaze.hpp>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"

namespace inspector
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
    template <class K, class V> struct IsMap<std::unordered_map<K, V>> : std::true_type
    {
    };

    template <class T> Edited draw(std::string_view name, T &value);

    template <class T, class Visit> void forEachNamedField(T &value, Visit &&visit)
    {
        constexpr auto Fields = glz::reflect<T>::size;
        [&]<std::size_t... I>(std::index_sequence<I...>)
        {
            (visit(glz::reflect<T>::keys[I], glz::get<I>(glz::to_tie(value))), ...);
        }(std::make_index_sequence<Fields>{});
    }

    template <class K> std::string keyLabel(const K &key)
    {
        if constexpr (std::is_same_v<K, std::string>)
            return key;
        else
            return std::to_string(key);
    }

    inline std::string labelled(std::string_view name)
    {
        return std::string("##") + std::string(name);
    }

    inline Edited drawNamed(std::string_view name, float &value)
    {
        ImGui::TextUnformatted(name.data(), name.data() + name.size());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragFloat(labelled(name).c_str(), &value, 0.5f));
    }

    inline Edited drawNamed(std::string_view name, int &value)
    {
        ImGui::TextUnformatted(name.data(), name.data() + name.size());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragInt(labelled(name).c_str(), &value));
    }

    inline Edited drawNamed(std::string_view name, bool &value)
    {
        return justEdited(ImGui::Checkbox(std::string(name).c_str(), &value));
    }

    inline Edited drawNamed(std::string_view name, std::string &value)
    {
        std::array<char, 256> buffer{};
        value.copy(buffer.data(), std::min(value.size(), buffer.size() - 1));

        ImGui::TextUnformatted(name.data(), name.data() + name.size());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool changed = ImGui::InputText(labelled(name).c_str(), buffer.data(), buffer.size());
        Edited edited = justEdited(changed);
        if (changed)
            value = buffer.data();

        return edited;
    }

    inline Edited drawNamed(std::string_view name, glm::vec2 &value)
    {
        ImGui::TextUnformatted(name.data(), name.data() + name.size());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragFloat2(labelled(name).c_str(), &value.x, 0.5f));
    }

    inline Edited drawNamed(std::string_view name, glm::ivec2 &value)
    {
        ImGui::TextUnformatted(name.data(), name.data() + name.size());
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragInt2(labelled(name).c_str(), &value.x));
    }

    template <class T> Edited drawUnder(std::string_view name, T &value)
    {
        if (!ImGui::TreeNode(std::string(name).c_str()))
            return {};

        Edited edited;
        forEachNamedField(
            value,
            [&edited](std::string_view fieldName, auto &field)
            { edited |= draw(fieldName, field); });

        ImGui::TreePop();
        return edited;
    }

    template <class T> Edited draw(std::string_view name, T &value)
    {
        if constexpr (HasCustomField<T>)
            return drawCustomField(name, value);
        else if constexpr (
            std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, int> ||
            std::is_same_v<T, std::string> || std::is_same_v<T, glm::vec2> ||
            std::is_same_v<T, glm::ivec2>)
            return drawNamed(name, value);
        else if constexpr (IsOptional<T>::value)
        {
            bool present = value.has_value();
            bool toggled = ImGui::Checkbox(std::string(name).c_str(), &present);
            Edited edited = justEdited(toggled);
            if (toggled)
                value = present ? std::optional(typename T::value_type{}) : std::nullopt;

            if (value)
            {
                ImGui::Indent();
                ImGui::PushID("value");
                edited |= draw(name, *value);
                ImGui::PopID();
                ImGui::Unindent();
            }

            return edited;
        }
        else if constexpr (IsVector<T>::value)
        {
            if (!ImGui::TreeNode(std::string(name).c_str()))
                return {};

            Edited edited;
            std::optional<std::size_t> takeAway;
            for (std::size_t index = 0; index < value.size(); ++index)
            {
                ImGui::PushID(static_cast<int>(index));
                if (ImGui::SmallButton("-"))
                    takeAway = index;

                ImGui::SameLine();
                edited |= draw(std::to_string(index), value[index]);
                ImGui::PopID();
            }

            bool addAsked = ImGui::SmallButton("+");

            if (takeAway)
                value.erase(value.begin() + static_cast<std::ptrdiff_t>(*takeAway));
            else if (addAsked)
                value.emplace_back();

            if (takeAway || addAsked)
                edited |= Edited{true, true};

            ImGui::TreePop();
            return edited;
        }
        else if constexpr (IsMap<T>::value)
        {
            if (!ImGui::TreeNode(std::string(name).c_str()))
                return {};

            Edited edited;
            for (auto &[key, entry] : value)
            {
                std::string label = keyLabel(key);
                ImGui::PushID(label.c_str());
                edited |= draw(label, entry);
                ImGui::PopID();
            }

            ImGui::TreePop();
            return edited;
        }
        else if constexpr (glz::reflectable<T>)
            return drawUnder(name, value);
        else
        {
            ImGui::TextDisabled("%s", std::string(name).c_str());
            return {};
        }
    }

    template <class T> Edited drawFields(T &value)
    {
        Edited edited;
        forEachNamedField(
            value,
            [&edited](std::string_view fieldName, auto &field)
            { edited |= draw(fieldName, field); });

        return edited;
    }
}
