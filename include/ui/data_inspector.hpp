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
#include <variant>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <glaze/glaze.hpp>
#include "ui/inspector_edited.hpp"
#include "ui/inspector_fields.hpp"
#include "ui/marked_label.hpp"
#include "ui/saved_in_scope.hpp"

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

    template <class T> struct IsVariant : std::false_type
    {
    };
    template <class... Alternatives>
    struct IsVariant<std::variant<Alternatives...>> : std::true_type
    {
    };

    template <class Variant, std::size_t... I>
    Variant alternativeAt(std::size_t chosen, std::index_sequence<I...>)
    {
        Variant holding;
        ((I == chosen ? (holding = std::variant_alternative_t<I, Variant>{}, 0) : 0), ...);
        return holding;
    }

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

    inline Edited drawNamed(std::string_view name, float &value, bool changed)
    {
        drawLabel(name, changed);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragFloat(labelled(name).c_str(), &value, 0.5f));
    }

    inline Edited drawNamed(std::string_view name, int &value, bool changed)
    {
        drawLabel(name, changed);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragInt(labelled(name).c_str(), &value));
    }

    inline Edited drawNamed(std::string_view name, bool &value, bool changed)
    {
        Marked marked(changed);
        return justEdited(ImGui::Checkbox(std::string(name).c_str(), &value));
    }

    inline Edited drawNamed(std::string_view name, std::string &value, bool changed)
    {
        std::array<char, 256> buffer{};
        value.copy(buffer.data(), std::min(value.size(), buffer.size() - 1));

        drawLabel(name, changed);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        bool typed = ImGui::InputText(labelled(name).c_str(), buffer.data(), buffer.size());
        Edited edited = justEdited(typed);
        if (typed)
            value = buffer.data();

        return edited;
    }

    inline Edited drawNamed(std::string_view name, glm::vec2 &value, bool changed)
    {
        drawLabel(name, changed);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragFloat2(labelled(name).c_str(), &value.x, 0.5f));
    }

    inline Edited drawNamed(std::string_view name, glm::ivec2 &value, bool changed)
    {
        drawLabel(name, changed);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-FLT_MIN);
        return justEdited(ImGui::DragInt2(labelled(name).c_str(), &value.x));
    }

    template <class T> Edited drawUnder(std::string_view name, T &value, bool changed)
    {
        bool open = false;
        {
            Marked marked(changed);
            open = ImGui::TreeNode(std::string(name).c_str());
        }

        if (!open)
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
        InField here(name);
        const bool changed = changedHere(value);
        Marking marking(changed);

        if constexpr (HasCustomField<T>)
            return drawCustomField(name, value);
        else if constexpr (
            std::is_same_v<T, bool> || std::is_same_v<T, float> || std::is_same_v<T, int> ||
            std::is_same_v<T, std::string> || std::is_same_v<T, glm::vec2> ||
            std::is_same_v<T, glm::ivec2>)
            return drawNamed(name, value, changed);
        else if constexpr (IsOptional<T>::value)
        {
            bool present = value.has_value();
            bool toggled = false;
            {
                Marked marked(changed);
                toggled = ImGui::Checkbox(std::string(name).c_str(), &present);
            }
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
            bool open = false;
            {
                Marked marked(changed);
                open = ImGui::TreeNode(std::string(name).c_str());
            }

            if (!open)
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
        else if constexpr (IsVariant<T>::value && glz::tagged<T>)
        {
            constexpr auto &Ids = glz::meta<T>::ids;
            Edited edited;
            drawLabel(name, changed);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-FLT_MIN);
            if (ImGui::BeginCombo(labelled(name).c_str(), Ids[value.index()]))
            {
                for (std::size_t alternative = 0; alternative < Ids.size(); ++alternative)
                    if (ImGui::Selectable(Ids[alternative], alternative == value.index()) &&
                        alternative != value.index())
                    {
                        value = alternativeAt<T>(
                            alternative, std::make_index_sequence<std::variant_size_v<T>>{});
                        edited |= Edited{true, true};
                    }

                ImGui::EndCombo();
            }

            ImGui::Indent();
            ImGui::PushID("held");
            std::visit(
                [&edited](auto &held)
                {
                    forEachNamedField(
                        held,
                        [&edited](std::string_view fieldName, auto &field)
                        { edited |= draw(fieldName, field); });
                },
                value);
            ImGui::PopID();
            ImGui::Unindent();
            return edited;
        }
        else if constexpr (IsMap<T>::value)
        {
            bool open = false;
            {
                Marked marked(changed);
                open = ImGui::TreeNode(std::string(name).c_str());
            }

            if (!open)
                return {};

            Edited edited;
            std::optional<typename T::key_type> takeAway;
            for (auto &[key, entry] : value)
            {
                std::string label = keyLabel(key);
                ImGui::PushID(label.c_str());
                if constexpr (std::is_same_v<typename T::key_type, std::string>)
                {
                    if (ImGui::SmallButton("-"))
                        takeAway = key;

                    ImGui::SameLine();
                }

                edited |= draw(label, entry);
                ImGui::PopID();
            }

            if constexpr (std::is_same_v<typename T::key_type, std::string>)
            {
                if (ImGui::SmallButton("+"))
                    ImGui::OpenPopup("##addNamed");

                if (ImGui::BeginPopup("##addNamed"))
                {
                    static std::array<char, 64> asked{};
                    ImGui::SetNextItemWidth(120.0f);
                    ImGui::InputTextWithHint("##name", "name", asked.data(), asked.size());
                    std::string wanted = asked.data();
                    ImGui::SameLine();
                    ImGui::BeginDisabled(wanted.empty() || value.contains(wanted));
                    if (ImGui::Button("add"))
                    {
                        value.emplace(wanted, typename T::mapped_type{});
                        asked.fill(0);
                        edited |= Edited{true, true};
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndDisabled();
                    ImGui::EndPopup();
                }

                if (takeAway)
                {
                    value.erase(*takeAway);
                    edited |= Edited{true, true};
                }
            }

            ImGui::TreePop();
            return edited;
        }
        else if constexpr (glz::reflectable<T>)
            return drawUnder(name, value, changed);
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

    template <class T> Edited drawFieldsExcept(T &value, std::string_view leftOut)
    {
        Edited edited;
        forEachNamedField(
            value,
            [&edited, leftOut](std::string_view fieldName, auto &field)
            {
                if (fieldName != leftOut)
                    edited |= draw(fieldName, field);
            });

        return edited;
    }
}
