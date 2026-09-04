#pragma once

#include <string>
#include <vector>

struct SheetData;

void checkFramesFit(
    const std::vector<int> &frames,
    const SheetData &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight);
