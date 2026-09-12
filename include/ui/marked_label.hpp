#pragma once

#include <optional>
#include <string_view>
#include <imgui.h>

namespace inspector
{
    class Marked
    {
    public:
        explicit Marked(bool changed);
        Marked(bool changed, bool refused);
        ~Marked();
        Marked(const Marked &) = delete;
        Marked &operator=(const Marked &) = delete;
        Marked(Marked &&) = delete;
        Marked &operator=(Marked &&) = delete;

    private:
        std::optional<ImVec4> marked;
    };

    class Marking
    {
    public:
        explicit Marking(bool changed);
        ~Marking();
        Marking(const Marking &) = delete;
        Marking &operator=(const Marking &) = delete;
        Marking(Marking &&) = delete;
        Marking &operator=(Marking &&) = delete;

    private:
        bool before = false;
    };

    bool markedHere();

    void drawLabel(std::string_view name, bool changed);
    void drawLabel(std::string_view name, bool changed, bool refused);
    void drawLabel(std::string_view name);

    void drawRefusal(std::string_view why);

    bool drawFold(std::string_view name, bool changed);
    bool drawFold(std::string_view name);
}
