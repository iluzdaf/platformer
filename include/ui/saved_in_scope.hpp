#pragma once

#include <string>
#include <string_view>
#include <glaze/glaze.hpp>
#include "serialization/only_what_differs.hpp"

template <class T> std::string asItWasSaved(const std::string &savedJson)
{
    T asSaved{};
    if (glz::read_json(asSaved, savedJson))
        return {};

    return differs::compact(asSaved);
}

class SavedInScope
{
public:
    explicit SavedInScope(const std::string &savedJson);
    ~SavedInScope();
    SavedInScope(const SavedInScope &) = delete;
    SavedInScope &operator=(const SavedInScope &) = delete;
    SavedInScope(SavedInScope &&) = delete;
    SavedInScope &operator=(SavedInScope &&) = delete;
};

namespace inspector
{
    class InField
    {
    public:
        explicit InField(std::string_view name);
        ~InField();
        InField(const InField &) = delete;
        InField &operator=(const InField &) = delete;
        InField(InField &&) = delete;
        InField &operator=(InField &&) = delete;
    };

    bool changedFromSaved(const std::string &nowJson);

    template <class T> bool changedHere(const T &value)
    {
        return changedFromSaved(differs::compact(value));
    }
}
