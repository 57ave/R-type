/*
** EPITECH PROJECT, 2025
** Logger
** File description:
** rtype
*/

#ifndef _CORE_LOGGER_
    #define _CORE_LOGGER_
    #include <string>
    #include <iostream>

    namespace rtype {
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
    } // namespace rtype

#endif /* !_CORE_LOGGER_ */
