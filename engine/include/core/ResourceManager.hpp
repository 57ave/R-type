/*
** EPITECH PROJECT, 2025
** RessourceManager
** File description:
** rtype
*/

#ifndef _RESSOURCEMANAGER_CACHE_SYSTEME_
    #define _RESSOURCEMANAGER_CACHE_SYSTEME_
    #include "ITexture.hpp"
    #include "ISprite.hpp"
    #include <string>
    #include <unordered_map>
    #include <memory>
    #include <functional>

    namespace rtype {
        namespace core {

            class ResourceManager {
                public:
                    // Texture management
                    void loadTexture(const std::string& path);
                    std::shared_ptr<rtype::engine::rendering::ITexture> getTexture(const std::string& path);
                    void unloadTexture(const std::string& path);
                    
                    // Sprite management
                    void loadSprite(const std::string& path);
                    std::shared_ptr<rtype::engine::rendering::ISprite> getSprite(const std::string& path);
                    void unloadSprite(const std::string& path);
                    
                    // Clear all caches
                    void clear();
                    
                    // Set factory functions for creating concrete implementations
                    void setTextureFactory(std::function<std::shared_ptr<rtype::engine::rendering::ITexture>()> factory);
                    void setSpriteFactory(std::function<std::shared_ptr<rtype::engine::rendering::ISprite>()> factory);

                protected:
                private:
                    std::unordered_map<std::string, std::shared_ptr<rtype::engine::rendering::ITexture>> _CacheTextures;
                    std::unordered_map<std::string, std::shared_ptr<rtype::engine::rendering::ISprite>> _CacheSprites;
                    
                    std::function<std::shared_ptr<rtype::engine::rendering::ITexture>()> _textureFactory;
                    std::function<std::shared_ptr<rtype::engine::rendering::ISprite>()> _spriteFactory;
            };

        } // namespace core
    } // namespace rtype

#endif /* !_RESSOURCEMANAGER_CACHE_SYSTEME_ */
