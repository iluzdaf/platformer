#pragma once

class ImGuiManager;
class Health;
class Texture2D;
struct HealthIconData;

void drawHealth(
    const ImGuiManager &imGuiManager,
    const Health &health,
    const Texture2D &icon,
    const HealthIconData &healthIcon);
