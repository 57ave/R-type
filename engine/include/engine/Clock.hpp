#ifndef RTYPE_ENGINE_CLOCK_HPP
#define RTYPE_ENGINE_CLOCK_HPP

#include <memory>

namespace rtype {
    namespace engine {

        // Forward declaration
        namespace internal {
            class ClockImpl;
        }

        // Clock class - measures elapsed time
        class Clock {
        public:
            Clock();
            ~Clock();

            // Get elapsed time in seconds and restart the clock
            float restart();

            // Get elapsed time in seconds without restarting
            float getElapsedTime() const;

        private:
            std::unique_ptr<internal::ClockImpl> m_impl;
        };

    } // namespace engine
} // namespace rtype

#endif // RTYPE_ENGINE_CLOCK_HPP
