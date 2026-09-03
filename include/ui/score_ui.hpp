#pragma once

class ImGuiManager;
class Score;
class Texture2D;
struct ScoreIcon;

void drawScore(
    const ImGuiManager &imGuiManager,
    const Score &score,
    const Texture2D &icon,
    const ScoreIcon &scoreIcon);
