#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "rendering/texture2d.hpp"

class TextureCache
{
public:
    void warm(const std::string &texturePath);
    void reload(const std::string &texturePath);

    const Texture2D &get(const std::string &texturePath) const;
    const Texture2D *find(const std::string &texturePath) const;

private:
    std::unordered_map<std::string, std::unique_ptr<Texture2D>> textures;
};
