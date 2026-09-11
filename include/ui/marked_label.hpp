#pragma once

#include <string_view>

namespace inspector
{
    class Marked
    {
    public:
        explicit Marked(bool changed);
        ~Marked();
        Marked(const Marked &) = delete;
        Marked &operator=(const Marked &) = delete;
        Marked(Marked &&) = delete;
        Marked &operator=(Marked &&) = delete;

    private:
        bool marked = false;
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
    void drawLabel(std::string_view name);
}
