#ifndef RTYPE_CORE_TIME_HPP
    #define RTYPE_CORE_TIME_HPP
    #include <chrono>

    namespace rtype {
        namespace core {

            // Time class to manage time-related functionalities
            class Time {
            public:
                // Constructor to initialize the start time
                Time() : _startTime(std::chrono::high_resolution_clock::now()) {}

                // Method to get the elapsed time in milliseconds
                long long getElapsedTime();

                // Method to reset the start time to the current time
                void reset();
                void update();
                float getDeltaTime() const {
                    return _deltaTime;
                }
                float getTotalTime() const {
                    return _totalTime;
                }
                float getTimeScale() const {
                    return _timeScale;
                }
                void setTimeScale(float timeScale) {
                    _timeScale = timeScale;
                }

            private:
                std::chrono::high_resolution_clock::time_point _startTime; // Start time point
                float _deltaTime; // Delta time in seconds
                float _totalTime; // Total time in seconds
                float _timeScale; // Time scale factor
            };

        } // namespace core
    } // namespace rtype
#endif // RTYPE_CORE_TIME_HPP