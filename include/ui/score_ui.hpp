#pragma once

class ImGuiManager;
class ScoringSystem;
class Texture2D;
struct ScoreIcon;

void drawScore(
    const ImGuiManager &imGuiManager,
    const ScoringSystem &scoringSystem,
    const Texture2D &icon,
    const ScoreIcon &scoreIcon);
