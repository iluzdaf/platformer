#pragma once
class ImGuiManager;
class ScoringSystem;
class Texture2D;

class ScoreUi
{
public:
    void draw(
        const ImGuiManager &imGuiManager,
        const ScoringSystem &scoringSystem,
        const Texture2D &tileSet);
};