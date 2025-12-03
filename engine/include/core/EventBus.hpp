/*
** EPITECH PROJECT, 2025
** Eventbus
** File description:
** rtype
*/

#ifndef EVENTBUS_CORE
    #define EVENTBUS_CORE

    template<typename T>
    class EventBus {
        public:
            EventBus();
            ~EventBus();
            subscribe<T>(callback);
            publish<T>(event);
            unsubscribe<T>(handle);

        protected:
        private:
    };

#endif /* !EVENTBUS_CORE */
