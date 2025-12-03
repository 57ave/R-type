/*
** EPITECH PROJECT, 2025
** RessourceManager
** File description:
** rtype
*/

#ifndef _RESSOURCEMANAGER_CACHE_SYSTEME_
    #define _RESSOURCEMANAGER_CACHE_SYSTEME_
    #include <string>
    #include <unordered_map>
    #include <memory>

template<typename T>
class ResourceManager {
    public:
        load(std::string path); // ne retourne rien, juste charge
        get(std::string path) -> std::shared_ptr<T>; // retourne la ressource
        unload(std::string path); // unload la ressource

    protected:
    private:
        std::unordered_map<std::string, std::shared_ptr<T>> _resourceCache;
};

#endif /* !_RESSOURCEMANAGER_CACHE_SYSTEME_ */
