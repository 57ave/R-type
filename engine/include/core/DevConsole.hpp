/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** In-game developer console for commands, debugging, and runtime configuration
*/

#ifndef RTYPE_CORE_DEV_CONSOLE_HPP
#define RTYPE_CORE_DEV_CONSOLE_HPP

#include <string>
#include <vector>
#include <deque>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <SFML/Graphics.hpp>

namespace rtype {
namespace core {

#ifdef _WIN32
    #undef INFO
    #undef ERROR
    #undef WARNING
    #undef SUCCESS
    #undef SYSTEM
    #undef COMMAND
#endif
enum class ConsoleMessageType {
    INFO,
    SUCCESS,
    WARNING,
    ERROR,
    COMMAND,
    SYSTEM
};

/**
 * @brief A single console message with type and timestamp
 */
struct ConsoleMessage {
    std::string text;
    ConsoleMessageType type;
    std::string timestamp;
};

/**
 * @brief Command callback function type
 * @param args Vector of arguments (first is command name)
 * @return Result message to display
 */
using CommandCallback = std::function<std::string(const std::vector<std::string>& args)>;

/**
 * @brief Command definition with name, help, and callback
 */
struct ConsoleCommand {
    std::string name;
    std::string description;
    std::string usage;
    CommandCallback callback;
};

/**
 * @brief In-game developer console
 * 
 * Features:
 * - Toggle with ` (backtick) or F1
 * - Command input with history (up/down arrows)
 * - Autocomplete with Tab
 * - Built-in commands (help, clear, quit, etc.)
 * - Custom command registration
 * - Scrollable log output
 * - Integration with Logger
 */
class DevConsole {
public:
    DevConsole();
    ~DevConsole() = default;
    
    /**
     * @brief Initialize the console with a font
     * @param fontPath Path to a .ttf font file (empty for auto-detect)
     * @return true if initialization succeeded
     */
    bool init(const std::string& fontPath = "");
    
    /**
     * @brief Initialize with an already-loaded font
     * @param font SFML font to use
     */
    void init(const sf::Font& font);
    
    /**
     * @brief Update the console state
     * @param deltaTime Time since last frame
     */
    void update(float deltaTime);
    
    /**
     * @brief Render the console to the window
     * @param window SFML RenderWindow to draw to
     */
    void render(sf::RenderWindow& window);
    
    /**
     * @brief Handle input events
     * @param event SFML event to process
     * @return true if the event was consumed by the console
     */
    bool handleEvent(const sf::Event& event);
    
    // ========================================
    // Visibility
    // ========================================
    void toggle();
    void open();
    void close();
    bool isOpen() const;
    
    // ========================================
    // Commands
    // ========================================
    
    /**
     * @brief Register a custom command
     * @param name Command name (no spaces)
     * @param description Short description for help
     * @param usage Usage example
     * @param callback Function to call when command is executed
     */
    void registerCommand(const std::string& name, 
                         const std::string& description,
                         const std::string& usage,
                         CommandCallback callback);
    
    /**
     * @brief Execute a command string
     * @param command The full command string to execute
     */
    void execute(const std::string& command);
    
    /**
     * @brief Unregister a command
     * @param name Command name to remove
     */
    void unregisterCommand(const std::string& name);
    
    // ========================================
    // Output
    // ========================================
    
    /**
     * @brief Print a message to the console
     * @param message The message to display
     * @param type Message type (affects color)
     */
    void print(const std::string& message, ConsoleMessageType type = ConsoleMessageType::INFO);
    
    /**
     * @brief Print formatted info message
     */
    void info(const std::string& message);
    
    /**
     * @brief Print formatted success message
     */
    void success(const std::string& message);
    
    /**
     * @brief Print formatted warning message
     */
    void warning(const std::string& message);
    
    /**
     * @brief Print formatted error message
     */
    void error(const std::string& message);
    
    /**
     * @brief Clear all console messages
     */
    void clear();
    
    // ========================================
    // Configuration
    // ========================================
    void setMaxMessages(size_t max);
    void setMaxHistory(size_t max);
    void setHeight(float height); // As percentage of window height (0.0-1.0)
    void setOpacity(float opacity);
    void setFontSize(unsigned int size);
    void handleTextInput(uint32_t unicode);
    void handleSpecialKey(sf::Keyboard::Key key);

    
private:
    void registerBuiltinCommands();
    void submitCommand();
    void historyUp();
    void historyDown();
    void autocomplete();
    std::vector<std::string> parseCommand(const std::string& input) const;
    std::vector<std::string> getMatchingCommands(const std::string& prefix) const;
    sf::Color getMessageColor(ConsoleMessageType type) const;
    std::string getTimestamp() const;
    void scrollUp();
    void scrollDown();
    void scrollToBottom();
    
private:
    bool _initialized = false;
    bool _isOpen = false;
    float _animationProgress = 0.0f; // 0 = closed, 1 = fully open
    float _animationSpeed = 8.0f;
    
    // Font and text
    sf::Font _font;
    bool _fontLoaded = false;
    unsigned int _fontSize = 14;
    
    // Input
    std::string _inputBuffer;
    size_t _cursorPosition = 0;
    float _cursorBlinkTimer = 0.0f;
    bool _cursorVisible = true;
    
    // Command history
    std::deque<std::string> _commandHistory;
    int _historyIndex = -1;
    size_t _maxHistory = 100;
    std::string _savedInput; // Save input when navigating history
    
    // Messages
    std::deque<ConsoleMessage> _messages;
    size_t _maxMessages = 500;
    int _scrollOffset = 0;
    
    // Commands
    std::unordered_map<std::string, ConsoleCommand> _commands;
    
    // Appearance
    float _height = 0.4f; // 40% of window height
    float _opacity = 0.9f;
    
    // Graphics
    sf::RectangleShape _background;
    sf::RectangleShape _inputBackground;
    sf::Text _inputText;
    sf::Text _cursorText;
    sf::Text _promptText;
    std::vector<sf::Text> _messageTexts;
    
    // Colors
    sf::Color _bgColor{20, 20, 30, 230};
    sf::Color _inputBgColor{30, 30, 40, 255};
    sf::Color _inputTextColor{255, 255, 255, 255};
    sf::Color _promptColor{100, 200, 100, 255};
    
    // Thread safety
    mutable std::mutex _mutex;
};

} // namespace core
} // namespace rtype

#endif // RTYPE_CORE_DEV_CONSOLE_HPP
