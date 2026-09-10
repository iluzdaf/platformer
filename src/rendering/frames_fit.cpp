#include <stdexcept>
#include <string>
#include "rendering/frames_fit.hpp"
#include <vector>
#include "assets/sheet_data.hpp"
#include "animations/frame_animation_data.hpp"
#include <cstddef>

void checkFramesFit(
    const std::vector<int> &frames,
    const SheetData &sheet,
    const std::string &whose,
    int textureWidth,
    int textureHeight)
{
    if (sheet.cellSize.x <= 0 || sheet.cellSize.y <= 0)
        throw std::runtime_error(whose + " has cells no wider or taller than nothing");

    int cells = (textureWidth / sheet.cellSize.x) * (textureHeight / sheet.cellSize.y);
    for (int frame : frames)
        if (frame < 0 || frame >= cells)
            throw std::runtime_error(
                whose + " animates on frame " + std::to_string(frame) + ", and \"" +
                sheet.texture.path + "\" holds " + std::to_string(cells));
}

void checkCuesFit(const FrameAnimationData &clip, const std::string &whose)
{
    for (const FrameCueData &cue : clip.cues)
        if (cue.frame < 0 || static_cast<std::size_t>(cue.frame) >= clip.frames.size())
            throw std::runtime_error(
                whose + " cues \"" + cue.name + "\" on frame " + std::to_string(cue.frame) +
                " of a clip " + std::to_string(clip.frames.size()) + " frames long");
}
