/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** In-game developer console implementation
*/

#include "core/DevConsole.hpp"
#include "core/Logger.hpp"
#include "core/Profiler.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>
#include <ctime>

namespace rtype {
namespace core {

DevConsole::DevConsole()
{
    _background.setFillColor(_bgColor);
    _inputBackground.setFillColor(_inputBgColor);
}

bool DevConsole::init(const std::string& fontPath)
{
    if (fontPath.empty()) {
        // Try common system font locations
        std::vector<std::string> fontPaths = {
            // macOS
            "/System/Library/Fonts/Menlo.ttc",
            "/System/Library/Fonts/Monaco.ttf",
            "/System/Library/Fonts/SFNSMono.ttf",
            "/Library/Fonts/Arial.ttf",
            // Linux
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
            // Windows
            "C:/Windows/Fonts/consola.ttf",
            "C:/Windows/Fonts/arial.ttf",
            // Relative paths
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "../../assets/fonts/arial.ttf"
        };
        
        for (const auto& path : fontPaths) {
            if (_font.loadFromFile(path)) {
                _fontLoaded = true;
                LOG_DEBUG("CONSOLE", "Loaded font from: " + path);
                break;
            }
        }
        
        if (!_fontLoaded) {
            LOG_WARNING("CONSOLE", "Could not load any system font - console text disabled");
            _initialized = true;
            return true;
        }
    } else {
        if (!_font.loadFromFile(fontPath)) {
            LOG_ERROR("CONSOLE", "Failed to load font: " + fontPath);
            return false;
        }
        _fontLoaded = true;
    }
    
    // Initialize text objects
    if (_fontLoaded) {
        _inputText.setFont(_font);
        _inputText.setCharacterSize(_fontSize);
        _inputText.setFillColor(_inputTextColor);
        
        _cursorText.setFont(_font);
        _cursorText.setCharacterSize(_fontSize);
        _cursorText.setFillColor(_inputTextColor);
        _cursorText.setString("_");
        
        _promptText.setFont(_font);
        _promptText.setCharacterSize(_fontSize);
        _promptText.setFillColor(_promptColor);
        _promptText.setString("> ");
    }
    
    registerBuiltinCommands();
    
    _initialized = true;
    print("Developer Console initialized. Type 'help' for commands.", ConsoleMessageType::SYSTEM);
    LOG_INFO("CONSOLE", "DevConsole initialized");
    return true;
}

void DevConsole::init(const sf::Font& font)
{
    _font = font;
    _fontLoaded = true;
    
    _inputText.setFont(_font);
    _inputText.setCharacterSize(_fontSize);
    _inputText.setFillColor(_inputTextColor);
    
    _cursorText.setFont(_font);
    _cursorText.setCharacterSize(_fontSize);
    _cursorText.setFillColor(_inputTextColor);
    _cursorText.setString("_");
    
    _promptText.setFont(_font);
    _promptText.setCharacterSize(_fontSize);
    _promptText.setFillColor(_promptColor);
    _promptText.setString("> ");
    
    registerBuiltinCommands();
    
    _initialized = true;
    print("Developer Console initialized. Type 'help' for commands.", ConsoleMessageType::SYSTEM);
}

void DevConsole::registerBuiltinCommands()
{
    // Help command
    registerCommand("help", "Display available commands", "help [command]",
        [this](const std::vector<std::string>& args) -> std::string {
            if (args.size() > 1) {
                auto it = _commands.find(args[1]);
                if (it != _commands.end()) {
                    return it->second.name + ": " + it->second.description + "\nUsage: " + it->second.usage;
                }
                return "Unknown command: " + args[1];
            }
            
            std::ostringstream ss;
            ss << "Available commands:\n";
            std::vector<std::string> names;
            for (const auto& [name, cmd] : _commands) {
                names.push_back(name);
            }
            std::sort(names.begin(), names.end());
            for (const auto& name : names) {
                ss << "  " << name << " - " << _commands[name].description << "\n";
            }
            return ss.str();
        });
    
    // Clear command
    registerCommand("clear", "Clear the console", "clear",
        [this](const std::vector<std::string>&) -> std::string {
            clear();
            return "";
        });
    
    // Quit command
    registerCommand("quit", "Exit the game", "quit",
        [](const std::vector<std::string>&) -> std::string {
            LOG_INFO("CONSOLE", "Quit command executed");
            // Note: actual quit needs to be handled by game loop
            return "Exiting...";
        });
    
    // Echo command
    registerCommand("echo", "Print a message", "echo <message>",
        [](const std::vector<std::string>& args) -> std::string {
            if (args.size() < 2) return "Usage: echo <message>";
            std::string result;
            for (size_t i = 1; i < args.size(); ++i) {
                if (i > 1) result += " ";
                result += args[i];
            }
            return result;
        });
    
    // FPS command
    registerCommand("fps", "Show current FPS", "fps",
        [](const std::vector<std::string>&) -> std::string {
            auto& profiler = Profiler::getInstance();
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(1);
            ss << "FPS: " << profiler.getCurrentFPS();
            ss << " (avg: " << profiler.getAverageFPS() << ")";
            ss << " Frame: " << profiler.getFrameTimeMs() << "ms";
            return ss.str();
        });
    
    // Stats command
    registerCommand("stats", "Show engine statistics", "stats",
        [](const std::vector<std::string>&) -> std::string {
            auto& profiler = Profiler::getInstance();
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2);
            ss << "--- Engine Stats ---\n";
            ss << "FPS: " << profiler.getCurrentFPS() << " (avg: " << profiler.getAverageFPS() << ")\n";
            ss << "Frame Time: " << profiler.getFrameTimeMs() << " ms\n";
            ss << "Entities: " << profiler.getEntityCount() << "\n";
            ss << "Draw Calls: " << profiler.getDrawCalls() << "\n";
            ss << "Memory: " << profiler.getMemoryUsageMB() << " MB";
            return ss.str();
        });
    
    // Log level command
    registerCommand("loglevel", "Set log level (debug/info/warning/error)", "loglevel <level>",
        [](const std::vector<std::string>& args) -> std::string {
            if (args.size() < 2) {
                auto level = Logger::getInstance().getMinLevel();
                std::string levelStr;
                switch (level) {
                    case LogLevel::OFF: levelStr = "OFF"; break;
                    case LogLevel::DEBUG: levelStr = "DEBUG"; break;
                    case LogLevel::INFO: levelStr = "INFO"; break;
                    case LogLevel::WARNING: levelStr = "WARNING"; break;
                    case LogLevel::ERROR: levelStr = "ERROR"; break;
                }
                return "Current log level: " + levelStr;
            }
            
            std::string levelStr = args[1];
            std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), ::tolower);
            
            LogLevel level;
            if (levelStr == "debug") level = LogLevel::DEBUG;
            else if (levelStr == "info") level = LogLevel::INFO;
            else if (levelStr == "warning") level = LogLevel::WARNING;
            else if (levelStr == "error") level = LogLevel::ERROR;
            else return "Invalid log level. Use: debug, info, warning, error";
            
            Logger::getInstance().setMinLevel(level);
            return "Log level set to: " + args[1];
        });
    
    // Profiler command
    registerCommand("profiler", "Toggle profiler overlay or show report", "profiler [on|off|report]",
        [](const std::vector<std::string>& args) -> std::string {
            auto& profiler = Profiler::getInstance();
            if (args.size() < 2) {
                return profiler.isEnabled() ? "Profiler is enabled" : "Profiler is disabled";
            }
            
            std::string action = args[1];
            std::transform(action.begin(), action.end(), action.begin(), ::tolower);
            
            if (action == "on") {
                profiler.setEnabled(true);
                return "Profiler enabled";
            } else if (action == "off") {
                profiler.setEnabled(false);
                return "Profiler disabled";
            } else if (action == "report") {
                return profiler.generateReport();
            } else if (action == "reset") {
                profiler.reset();
                return "Profiler stats reset";
            }
            
            return "Usage: profiler [on|off|report|reset]";
        });
    
    // History command
    registerCommand("history", "Show command history", "history",
        [this](const std::vector<std::string>&) -> std::string {
            if (_commandHistory.empty()) {
                return "No command history";
            }
            std::ostringstream ss;
            int num = 1;
            for (const auto& cmd : _commandHistory) {
                ss << num++ << ": " << cmd << "\n";
            }
            return ss.str();
        });
    
    // Version command
    registerCommand("version", "Show engine version", "version",
        [](const std::vector<std::string>&) -> std::string {
            return "R-Type Engine v1.0.0\nBuilt: " __DATE__ " " __TIME__;
        });
}

void DevConsole::update(float deltaTime)
{
    if (!_initialized) return;
    
    // Animate console open/close
    float targetProgress = _isOpen ? 1.0f : 0.0f;
    if (_animationProgress != targetProgress) {
        float diff = targetProgress - _animationProgress;
        _animationProgress += diff * _animationSpeed * deltaTime;
        
        // Clamp to target when close enough
        if (std::abs(diff) < 0.01f) {
            _animationProgress = targetProgress;
        }
    }
    
    // Cursor blink
    if (_isOpen) {
        _cursorBlinkTimer += deltaTime;
        if (_cursorBlinkTimer >= 0.5f) {
            _cursorBlinkTimer = 0.0f;
            _cursorVisible = !_cursorVisible;
        }
    }
}

void DevConsole::render(sf::RenderWindow& window)
{
    if (!_initialized || _animationProgress <= 0.0f) return;
    
    sf::Vector2u windowSize = window.getSize();
    float consoleHeight = windowSize.y * _height * _animationProgress;
    float lineHeight = _fontSize + 4;
    float padding = 8.0f;
    
    // Save current view
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    // Background
    sf::Color bgColor = _bgColor;
    bgColor.a = static_cast<uint8_t>(_opacity * 255 * _animationProgress);
    _background.setFillColor(bgColor);
    _background.setSize(sf::Vector2f(static_cast<float>(windowSize.x), consoleHeight));
    _background.setPosition(0, 0);
    window.draw(_background);
    
    // Input area background
    float inputAreaHeight = lineHeight + padding * 2;
    sf::Color inputBgColor = _inputBgColor;
    inputBgColor.a = static_cast<uint8_t>(_opacity * 255 * _animationProgress);
    _inputBackground.setFillColor(inputBgColor);
    _inputBackground.setSize(sf::Vector2f(static_cast<float>(windowSize.x), inputAreaHeight));
    _inputBackground.setPosition(0, consoleHeight - inputAreaHeight);
    window.draw(_inputBackground);
    
    if (_fontLoaded) {
        // Calculate visible area for messages
        float messagesAreaHeight = consoleHeight - inputAreaHeight - padding;
        int visibleLines = static_cast<int>(messagesAreaHeight / lineHeight);
        
        // Render messages (from bottom to top)
        float messageY = consoleHeight - inputAreaHeight - lineHeight - padding;
        int startIndex = static_cast<int>(_messages.size()) - 1 - _scrollOffset;
        int linesDrawn = 0;
        
        for (int i = startIndex; i >= 0 && linesDrawn < visibleLines; --i, ++linesDrawn) {
            const auto& msg = _messages[i];
            
            sf::Text msgText;
            msgText.setFont(_font);
            msgText.setCharacterSize(_fontSize);
            msgText.setFillColor(getMessageColor(msg.type));
            msgText.setString("[" + msg.timestamp + "] " + msg.text);
            msgText.setPosition(padding, messageY);
            
            window.draw(msgText);
            messageY -= lineHeight;
        }
        
        // Render input prompt and text
        float inputY = consoleHeight - inputAreaHeight + padding;
        
        _promptText.setPosition(padding, inputY);
        window.draw(_promptText);
        
        float promptWidth = _promptText.getLocalBounds().width + 5;
        
        _inputText.setString(_inputBuffer);
        _inputText.setPosition(padding + promptWidth, inputY);
        window.draw(_inputText);
        
        // Render cursor
        if (_cursorVisible && _isOpen) {
            std::string beforeCursor = _inputBuffer.substr(0, _cursorPosition);
            sf::Text measureText;
            measureText.setFont(_font);
            measureText.setCharacterSize(_fontSize);
            measureText.setString(beforeCursor);
            float cursorX = padding + promptWidth + measureText.getLocalBounds().width;
            
            _cursorText.setPosition(cursorX, inputY);
            window.draw(_cursorText);
        }
    }
    
    // Restore view
    window.setView(currentView);
}

bool DevConsole::handleEvent(const sf::Event& event)
{
    // Toggle console with backtick or F1
    if (event.type == sf::Event::KeyPressed) {
        // Backtick (`) or F1 to toggle
        if (event.key.code == sf::Keyboard::F1 || 
            (event.key.code == sf::Keyboard::Grave && !event.key.shift)) {
            toggle();
            return true;
        }
    }
    
    // If not open, don't consume events
    if (!_isOpen) {
        return false;
    }
    
    // Handle text input
    if (event.type == sf::Event::TextEntered) {
        handleTextInput(event.text.unicode);
        return true;
    }
    
    // Handle special keys
    if (event.type == sf::Event::KeyPressed) {
        handleSpecialKey(event.key.code);
        return true;
    }
    
    // Handle mouse wheel for scrolling
    if (event.type == sf::Event::MouseWheelScrolled) {
        if (event.mouseWheelScroll.delta > 0) {
            scrollUp();
        } else {
            scrollDown();
        }
        return true;
    }
    
    return true; // Consume all events when console is open
}

void DevConsole::handleTextInput(uint32_t unicode)
{
    // Ignore control characters and backtick
    if (unicode < 32 || unicode == 127 || unicode == '`') {
        return;
    }
    
    // Insert character at cursor position
    if (unicode < 128) {
        _inputBuffer.insert(_cursorPosition, 1, static_cast<char>(unicode));
        _cursorPosition++;
        _cursorVisible = true;
        _cursorBlinkTimer = 0.0f;
    }
}

void DevConsole::handleSpecialKey(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Return:
            submitCommand();
            break;
            
        case sf::Keyboard::BackSpace:
            if (_cursorPosition > 0) {
                _inputBuffer.erase(_cursorPosition - 1, 1);
                _cursorPosition--;
            }
            break;
            
        case sf::Keyboard::Delete:
            if (_cursorPosition < _inputBuffer.length()) {
                _inputBuffer.erase(_cursorPosition, 1);
            }
            break;
            
        case sf::Keyboard::Left:
            if (_cursorPosition > 0) {
                _cursorPosition--;
            }
            break;
            
        case sf::Keyboard::Right:
            if (_cursorPosition < _inputBuffer.length()) {
                _cursorPosition++;
            }
            break;
            
        case sf::Keyboard::Home:
            _cursorPosition = 0;
            break;
            
        case sf::Keyboard::End:
            _cursorPosition = _inputBuffer.length();
            break;
            
        case sf::Keyboard::Up:
            historyUp();
            break;
            
        case sf::Keyboard::Down:
            historyDown();
            break;
            
        case sf::Keyboard::Tab:
            autocomplete();
            break;
            
        case sf::Keyboard::PageUp:
            for (int i = 0; i < 5; ++i) scrollUp();
            break;
            
        case sf::Keyboard::PageDown:
            for (int i = 0; i < 5; ++i) scrollDown();
            break;
            
        case sf::Keyboard::Escape:
            close();
            break;
            
        default:
            break;
    }
    
    _cursorVisible = true;
    _cursorBlinkTimer = 0.0f;
}

void DevConsole::submitCommand()
{
    std::string command = _inputBuffer;
    _inputBuffer.clear();
    _cursorPosition = 0;
    _historyIndex = -1;
    
    // Trim whitespace
    size_t start = command.find_first_not_of(" \t");
    if (start == std::string::npos) {
        return; // Empty command
    }
    size_t end = command.find_last_not_of(" \t");
    command = command.substr(start, end - start + 1);
    
    // Add to history
    if (!command.empty()) {
        // Don't add duplicates to history
        if (_commandHistory.empty() || _commandHistory.back() != command) {
            _commandHistory.push_back(command);
            if (_commandHistory.size() > _maxHistory) {
                _commandHistory.pop_front();
            }
        }
    }
    
    // Print the command
    print("> " + command, ConsoleMessageType::COMMAND);
    
    // Execute
    execute(command);
    
    // Scroll to bottom
    scrollToBottom();
}

void DevConsole::execute(const std::string& command)
{
    auto args = parseCommand(command);
    if (args.empty()) {
        return;
    }
    
    std::string cmdName = args[0];
    std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::tolower);
    
    auto it = _commands.find(cmdName);
    if (it == _commands.end()) {
        error("Unknown command: " + cmdName + ". Type 'help' for available commands.");
        return;
    }
    
    try {
        std::string result = it->second.callback(args);
        if (!result.empty()) {
            // Split multi-line results
            std::istringstream iss(result);
            std::string line;
            while (std::getline(iss, line)) {
                print(line, ConsoleMessageType::SUCCESS);
            }
        }
    } catch (const std::exception& e) {
        error("Command error: " + std::string(e.what()));
    }
}

void DevConsole::historyUp()
{
    if (_commandHistory.empty()) return;
    
    if (_historyIndex == -1) {
        _savedInput = _inputBuffer;
        _historyIndex = static_cast<int>(_commandHistory.size()) - 1;
    } else if (_historyIndex > 0) {
        _historyIndex--;
    }
    
    _inputBuffer = _commandHistory[_historyIndex];
    _cursorPosition = _inputBuffer.length();
}

void DevConsole::historyDown()
{
    if (_historyIndex == -1) return;
    
    if (_historyIndex < static_cast<int>(_commandHistory.size()) - 1) {
        _historyIndex++;
        _inputBuffer = _commandHistory[_historyIndex];
    } else {
        _historyIndex = -1;
        _inputBuffer = _savedInput;
    }
    
    _cursorPosition = _inputBuffer.length();
}

void DevConsole::autocomplete()
{
    if (_inputBuffer.empty()) return;
    
    auto matches = getMatchingCommands(_inputBuffer);
    
    if (matches.empty()) {
        return;
    } else if (matches.size() == 1) {
        _inputBuffer = matches[0] + " ";
        _cursorPosition = _inputBuffer.length();
    } else {
        // Print all matches
        std::ostringstream ss;
        for (const auto& match : matches) {
            ss << match << "  ";
        }
        print(ss.str(), ConsoleMessageType::INFO);
        
        // Find common prefix
        std::string commonPrefix = matches[0];
        for (size_t i = 1; i < matches.size(); ++i) {
            size_t len = 0;
            while (len < commonPrefix.length() && len < matches[i].length() &&
                   commonPrefix[len] == matches[i][len]) {
                len++;
            }
            commonPrefix = commonPrefix.substr(0, len);
        }
        
        if (commonPrefix.length() > _inputBuffer.length()) {
            _inputBuffer = commonPrefix;
            _cursorPosition = _inputBuffer.length();
        }
    }
}

std::vector<std::string> DevConsole::parseCommand(const std::string& input) const
{
    std::vector<std::string> args;
    std::istringstream iss(input);
    std::string token;
    
    while (iss >> token) {
        args.push_back(token);
    }
    
    return args;
}

std::vector<std::string> DevConsole::getMatchingCommands(const std::string& prefix) const
{
    std::vector<std::string> matches;
    std::string lowerPrefix = prefix;
    std::transform(lowerPrefix.begin(), lowerPrefix.end(), lowerPrefix.begin(), ::tolower);
    
    for (const auto& [name, cmd] : _commands) {
        if (name.substr(0, lowerPrefix.length()) == lowerPrefix) {
            matches.push_back(name);
        }
    }
    
    std::sort(matches.begin(), matches.end());
    return matches;
}

sf::Color DevConsole::getMessageColor(ConsoleMessageType type) const
{
    switch (type) {
        case ConsoleMessageType::INFO:    return sf::Color(200, 200, 200);
        case ConsoleMessageType::SUCCESS: return sf::Color(100, 255, 100);
        case ConsoleMessageType::WARNING: return sf::Color(255, 200, 50);
        case ConsoleMessageType::ERROR:   return sf::Color(255, 80, 80);
        case ConsoleMessageType::COMMAND: return sf::Color(100, 200, 255);
        case ConsoleMessageType::SYSTEM:  return sf::Color(200, 100, 255);
        default: return sf::Color::White;
    }
}

std::string DevConsole::getTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time_t);
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

void DevConsole::scrollUp()
{
    int maxScroll = static_cast<int>(_messages.size()) - 5;
    if (_scrollOffset < maxScroll) {
        _scrollOffset++;
    }
}

void DevConsole::scrollDown()
{
    if (_scrollOffset > 0) {
        _scrollOffset--;
    }
}

void DevConsole::scrollToBottom()
{
    _scrollOffset = 0;
}

void DevConsole::toggle()
{
    _isOpen = !_isOpen;
    if (_isOpen) {
        _cursorVisible = true;
        _cursorBlinkTimer = 0.0f;
    }
}

void DevConsole::open()
{
    _isOpen = true;
    _cursorVisible = true;
    _cursorBlinkTimer = 0.0f;
}

void DevConsole::close()
{
    _isOpen = false;
}

bool DevConsole::isOpen() const
{
    return _isOpen;
}

void DevConsole::registerCommand(const std::string& name,
                                  const std::string& description,
                                  const std::string& usage,
                                  CommandCallback callback)
{
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    
    _commands[lowerName] = ConsoleCommand{lowerName, description, usage, callback};
}

void DevConsole::unregisterCommand(const std::string& name)
{
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    _commands.erase(lowerName);
}

void DevConsole::print(const std::string& message, ConsoleMessageType type)
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    ConsoleMessage msg;
    msg.text = message;
    msg.type = type;
    msg.timestamp = getTimestamp();
    
    _messages.push_back(msg);
    
    if (_messages.size() > _maxMessages) {
        _messages.pop_front();
    }
}

void DevConsole::info(const std::string& message)
{
    print(message, ConsoleMessageType::INFO);
}

void DevConsole::success(const std::string& message)
{
    print(message, ConsoleMessageType::SUCCESS);
}

void DevConsole::warning(const std::string& message)
{
    print(message, ConsoleMessageType::WARNING);
}

void DevConsole::error(const std::string& message)
{
    print(message, ConsoleMessageType::ERROR);
}

void DevConsole::clear()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _messages.clear();
    _scrollOffset = 0;
}

void DevConsole::setMaxMessages(size_t max)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _maxMessages = max;
    while (_messages.size() > _maxMessages) {
        _messages.pop_front();
    }
}

void DevConsole::setMaxHistory(size_t max)
{
    _maxHistory = max;
    while (_commandHistory.size() > _maxHistory) {
        _commandHistory.pop_front();
    }
}

void DevConsole::setHeight(float height)
{
    _height = std::clamp(height, 0.1f, 0.9f);
}

void DevConsole::setOpacity(float opacity)
{
    _opacity = std::clamp(opacity, 0.0f, 1.0f);
}

void DevConsole::setFontSize(unsigned int size)
{
    _fontSize = size;
    if (_fontLoaded) {
        _inputText.setCharacterSize(_fontSize);
        _cursorText.setCharacterSize(_fontSize);
        _promptText.setCharacterSize(_fontSize);
    }
}

} // namespace core
} // namespace rtype
