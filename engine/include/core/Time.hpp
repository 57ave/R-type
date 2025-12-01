#ifndef RTYPE_CORE_TIME_HPP
    #define RTYPE_CORE_TIME_HPP
    #include <chrono>

    namespace rtype {
        namespace core {

            // Time class to manage time-related functionalities
            class Time {
            public:
                // Constructor to initialize the start time
                Time() : startTime(std::chrono::high_resolution_clock::now()) {}

                // Method to get the elapsed time in milliseconds
                long long getElapsedTime() const {
                    auto currentTime = std::chrono::high_resolution_clock::now();
                    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();
                }

                // Method to reset the start time to the current time
                void reset() {
                    startTime = std::chrono::high_resolution_clock::now();
                }

            private:
                std::chrono::high_resolution_clock::time_point startTime; // Start time point
            };

        } // namespace core
    } // namespace rtype
#endif // RTYPE_CORE_TIME_HPP