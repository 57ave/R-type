
#ifndef ENG_CORE_CONFIG_HPP
    #define ENG_CORE_CONFIG_HPP
    #include <iostream>
    #include <unordered_map>
    #include <string>
    #include <sstream>

    namespace eng {
        namespace core {

            // Configuration class to manage application settings
            class Config {
            public:
                Config();
                ~Config();

                // Charge les assets depuis un dossier (scanne les .gif automatiquement)
                void load(const std::string& filepath);
                
                // Récupère une valeur par clé
                template<typename T>
                T get(const std::string& key) const;
                
                // Définit une valeur
                template<typename T>
                void set(const std::string& key, const T& value);
                
                // Sauvegarde la configuration dans un fichier
                void save(const std::string& filepath) const;

            private:
                std::unordered_map<std::string, std::string> _data;
                
                // Helpers pour le scan automatique
                void scanDirectory(const std::string& path, const std::string& category);
                std::string getFilenameWithoutExtension(const std::string& filepath) const;
            };

            // ========================================
            // IMPLÉMENTATION DES TEMPLATES
            // ========================================
            
            template<typename T>
            T Config::get(const std::string& key) const {
                auto it = _data.find(key);
                if (it != _data.end()) {
                    std::istringstream iss(it->second);
                    T value;
                    iss >> value;
                    return value;
                }
                std::cerr << "Key not found: " << key << std::endl;
                return T();
            }

            // Spécialisation pour std::string
            template<>
            inline std::string Config::get<std::string>(const std::string& key) const {
                auto it = _data.find(key);
                if (it != _data.end()) {
                    return it->second;
                }
                std::cerr << "Key not found: " << key << std::endl;
                return "";
            }

            template<typename T>
            void Config::set(const std::string& key, const T& value) {
                std::ostringstream oss;
                oss << value;
                _data[key] = oss.str();
            }

            // Spécialisation pour std::string
            template<>
            inline void Config::set<std::string>(const std::string& key, const std::string& value) {
                _data[key] = value;
            }

        } // namespace core
    } // namespace eng

#endif // ENG_CORE_CONFIG_HPP