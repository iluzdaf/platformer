#pragma once

#include "assets/sheet_data.hpp"

class Texture2D;

struct SheetInScope
{
    const Texture2D *texture = nullptr;
    SheetData sheet;
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
