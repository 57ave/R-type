/*
** EPITECH PROJECT, 2025
** Logger
** File description:
** engine
*/

#ifndef _CORE_LOGGER_
    #define _CORE_LOGGER_
    #include <string>
    #include <iostream>

    namespace eng {
        namespace core {

            class Logger {
                public:
                    Logger();
                    ~Logger();
                    void info(const std::string& message);
                    void warning(const std::string& message);
                    void error(const std::string& message);
                    void debug(const std::string& message);

                protected:
                private:
                    // std::vector<ILogOutput*> _Outputs;
            };

        } // namespace core
    } // namespace eng

#endif /* !_CORE_LOGGER_ */
