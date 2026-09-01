#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include "rendering/texture_cache.hpp"
#include "rendering/texture2d.hpp"
#include "assets/asset_paths.hpp"

namespace
{
    std::unique_ptr<Texture2D> loadTexture(const std::string &texturePath)
    {
        try
        {
            return std::make_unique<Texture2D>(assets::pathTo(texturePath));
        }
        catch (const std::exception &)
        {
            throw std::runtime_error("Failed to load texture \"" + texturePath + "\"");
        }
    }
}

void TextureCache::warm(const std::string &texturePath)
{
    if (textures.contains(texturePath))
        return;

    textures.insert_or_assign(texturePath, loadTexture(texturePath));
}

void TextureCache::reload(const std::string &texturePath)
{
    if (!textures.contains(texturePath))
        return;

    std::unique_ptr<Texture2D> replacement = loadTexture(texturePath);
    textures.insert_or_assign(texturePath, std::move(replacement));
}

const Texture2D &TextureCache::get(const std::string &texturePath) const
{
    const Texture2D *texture = find(texturePath);
    if (!texture)
        throw std::runtime_error("No texture loaded for \"" + texturePath + "\"");

    return *texture;
}

const Texture2D *TextureCache::find(const std::string &texturePath) const
{
    auto found = textures.find(texturePath);

    return found == textures.end() ? nullptr : found->second.get();
}
