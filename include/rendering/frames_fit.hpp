#pragma once

#include <string>

struct FrameAnimationData;
struct Sheet;

void checkFramesFit(
    const FrameAnimationData &animation,
    const Sheet &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight);
