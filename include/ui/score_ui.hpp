#pragma once

class ImGuiManager;
class Score;
class Texture2D;
struct ScoreIconData;

void drawScore(
    const ImGuiManager &imGuiManager,
    const Score &score,
    const Texture2D &icon,
    const ScoreIconData &scoreIcon);
