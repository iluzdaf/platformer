#pragma once

#include <string>
#include <vector>

struct SheetData;
struct FrameAnimationData;

void checkFramesFit(
    const std::vector<int> &frames,
    const SheetData &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight);

void checkCuesFit(const FrameAnimationData &clip, const std::string &whose);
