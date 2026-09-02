#pragma once

#include <string>
#include <vector>

struct Sheet;

void checkFramesFit(
    const std::vector<int> &frames,
    const Sheet &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight);
