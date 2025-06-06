#pragma once
class GLFWwindow;
class Camera2D;
class TileMap;
class Player;

class DebugRenderer
{
public:
    DebugRenderer(
        GLFWwindow *window,
        float screenWidth,
        float screenHeight);
    ~DebugRenderer();
    void draw(
        const Camera2D &camera,
        const TileMap &tileMap,
        const Player &player);
    void resize(float screenWidth, float screenHeight);

private:
    float screenWidth = 800, screenHeight = 600;
};