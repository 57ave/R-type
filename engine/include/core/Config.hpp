
#ifndef RTYPE_CORE_CONFIG_HPP
    #define RTYPE_CORE_CONFIG_HPP
    #include <iostream>
    #include <unordered_map>

    namespace rtype {
        namespace core {

            // Configuration class to manage application settings
            class Config {
            public:
                // // Constructor to initialize default settings
                // Config() : settingA(true), settingB(10) {}

                // // Method to display current settings
                // void displaySettings() const {
                //     std::cout << "Setting A: " << (settingA ? "Enabled" : "Disabled") << std::endl;
                //     std::cout << "Setting B: " << settingB << std::endl;
                // }

                // // Method to update settings
                // void updateSettings(bool a, int b) {
                //     settingA = a;
                //     settingB = b;
                // }

                void load(std::string filepath);
                void get<T>(int key) -> T;
                void set<T>(int key, std::string value);
                void save(std::string filepath);

            private:
                std::unordered_map<std::string, std::string> _data;
            };

        } // namespace core
    } // namespace rtype

#endif // RTYPE_CORE_CONFIG_HPP