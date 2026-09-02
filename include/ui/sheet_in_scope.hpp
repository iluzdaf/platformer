#pragma once

#include "assets/sheet.hpp"

class Texture2D;

struct SheetInScope
{
    const Texture2D *texture = nullptr;
    Sheet sheet;
    int frame = 0;
};

const SheetInScope *sheetInScope();

class ShowingSheet
{
public:
    explicit ShowingSheet(const SheetInScope &scope);
    ~ShowingSheet();

    ShowingSheet(const ShowingSheet &) = delete;
    ShowingSheet &operator=(const ShowingSheet &) = delete;

private:
    const SheetInScope *before = nullptr;
};
