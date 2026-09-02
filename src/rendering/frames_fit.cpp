#include <stdexcept>
#include <string>
#include "rendering/frames_fit.hpp"
#include "animations/frame_animation_data.hpp"
#include "assets/sheet.hpp"

void checkFramesFit(
    const FrameAnimationData &animation,
    const Sheet &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight)
{
    if (sheet.cellSize.x <= 0 || sheet.cellSize.y <= 0)
        throw std::runtime_error(whose + " has cells no wider or taller than nothing");

    int cells = (textureWidth / sheet.cellSize.x) * (textureHeight / sheet.cellSize.y);
    for (int frame : animation.frames)
        if (frame < 0 || frame >= cells)
            throw std::runtime_error(
                whose + " animates on frame " + std::to_string(frame) + ", and \"" + sheet.texture +
                "\" holds " + std::to_string(cells));
}
