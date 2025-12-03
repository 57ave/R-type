/*
** EPITECH PROJECT, 2025
** Logger
** File description:
** rtype
*/

#ifndef _CORE_LOGGER_
    #define _CORE_LOGGER_
    #include <string>

    class Logger {
        public:
            Logger();
            ~Logger();
            void info(std::string message);
            void warning(std::string message);
            void error(std::string message);
            void debug(std::string message);


        protected:
        private:
            // std::vector<ILogOutput*> _Outputs;
    };

#endif /* !_CORE_LOGGER_ */
