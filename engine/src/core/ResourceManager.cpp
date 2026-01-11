/*
** EPITECH PROJECT, 2025
** ResourceManager
** File description:
** rtype
*/

#include "core/ResourceManager.hpp"
#include <stdexcept>

// Texture methods
void rtype::core::ResourceManager::loadTexture(const std::string& path) {
    // Check if already loaded
    if (_CacheTextures.find(path) != _CacheTextures.end()) {
        return;
    }

    // Create new texture using factory
    if (!_textureFactory) {
        throw std::runtime_error("Texture factory not set. Call setTextureFactory() first.");
    }

    auto texture = _textureFactory();
    if (!texture->loadFromFile(path)) {
        throw std::runtime_error("Failed to load texture from: " + path);
    }

    _CacheTextures[path] = texture;
}

std::shared_ptr<rtype::engine::rendering::ITexture> rtype::core::ResourceManager::getTexture(const std::string& path) {
    // Try to find in cache
    auto it = _CacheTextures.find(path);
    if (it != _CacheTextures.end()) {
        return it->second;
    }
    
    // Not found, load it first
    loadTexture(path);
    return _CacheTextures[path];
}

void rtype::core::ResourceManager::unloadTexture(const std::string& path) {
    _CacheTextures.erase(path);
}

// Sprite methods
void rtype::core::ResourceManager::loadSprite(const std::string& path) {
    // Check if already loaded
    if (_CacheSprites.find(path) != _CacheSprites.end()) {
        return;
    }
    
    // Create new sprite using factory
    if (!_spriteFactory) {
        throw std::runtime_error("Sprite factory not set. Call setSpriteFactory() first.");
    }
    
    auto sprite = _spriteFactory();
    _CacheSprites[path] = sprite;
}

std::shared_ptr<rtype::engine::rendering::ISprite> rtype::core::ResourceManager::getSprite(const std::string& path) {
    // Try to find in cache
    auto it = _CacheSprites.find(path);
    if (it != _CacheSprites.end()) {
        return it->second;
    }
    
    // Not found, load it first
    loadSprite(path);
    return _CacheSprites[path];
}

void rtype::core::ResourceManager::unloadSprite(const std::string& path) {
    _CacheSprites.erase(path);
}

// General methods
void rtype::core::ResourceManager::clear() {
    _CacheTextures.clear();
    _CacheSprites.clear();
}

void rtype::core::ResourceManager::setTextureFactory(std::function<std::shared_ptr<rtype::engine::rendering::ITexture>()> factory) {
    _textureFactory = factory;
}

void rtype::core::ResourceManager::setSpriteFactory(std::function<std::shared_ptr<rtype::engine::rendering::ISprite>()> factory) {
    _spriteFactory = factory;
}
