
#ifndef RTYPE_CORE_CONFIG_HPP
    #define RTYPE_CORE_CONFIG_HPP
    #include <iostream>

    namespace rtype {
        namespace core {

            // Configuration class to manage application settings
            class Config {
            public:
                // Constructor to initialize default settings
                Config() : settingA(true), settingB(10) {}

                // Method to display current settings
                void displaySettings() const {
                    std::cout << "Setting A: " << (settingA ? "Enabled" : "Disabled") << std::endl;
                    std::cout << "Setting B: " << settingB << std::endl;
                }

                // Method to update settings
                void updateSettings(bool a, int b) {
                    settingA = a;
                    settingB = b;
                }
            private:
                bool settingA; // Example boolean setting
                int settingB;  // Example integer setting
            };

        } // namespace core
    } // namespace rtype

#endif // RTYPE_CORE_CONFIG_HPP