#pragma once

/**
 * @file atlas_console.h
 * @brief In-engine developer console for the Atlas Engine
 *
 * Opened with the backtick (`) key. Provides a command-line interface
 * for modifying settings, forcing saves, inspecting game state, and
 * running diagnostic commands.
 *
 * The console overlays the top portion of the screen with a translucent
 * dark panel, a scrollable output log, and a single-line text input field.
 */

#include "atlas_context.h"
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>

namespace atlas {

class AtlasConsole {
public:
    /// Callback signature for console commands: (args) -> void
    using CommandHandler = std::function<void(const std::vector<std::string>& args)>;

    AtlasConsole();
    ~AtlasConsole() = default;

    // ── Visibility ──────────────────────────────────────────────────

    /** Toggle the console open/closed. */
    void toggle();

    /** Check if console is currently open. */
    bool isOpen() const { return m_open; }

    /** Set console open state directly. */
    void setOpen(bool open) { m_open = open; }

    // ── Input ───────────────────────────────────────────────────────

    /** Handle a character input event (for typing in the input field). */
    void handleChar(unsigned int codepoint);

    /** Handle a key press (Enter to submit, Backspace, Up/Down for history, Escape to close). */
    void handleKey(int key, int action);

    // ── Rendering ───────────────────────────────────────────────────

    /** Render the console overlay. Call between beginFrame/endFrame. */
    void render(AtlasContext& ctx);

    // ── Commands ────────────────────────────────────────────────────

    /** Register a named command with a handler. */
    void registerCommand(const std::string& name, CommandHandler handler,
                         const std::string& helpText = "");

    /** Print a line to the console output log. */
    void print(const std::string& text);

    /** Print a line with a specific color. */
    void print(const std::string& text, const Color& color);

    /** Get all output lines (read-only). */
    const std::vector<std::string>& getOutputLines() const { return m_outputText; }

    /** Clear the output log. */
    void clearOutput();

    // ── Built-in command registration ───────────────────────────────

    /** Register the default built-in commands (help, clear, quit, fps, set, save). */
    void registerBuiltinCommands();

    /** Set callback for "quit" command. */
    void setQuitCallback(std::function<void()> cb) { m_quitCb = std::move(cb); }

    /** Set callback for "save" command. */
    void setSaveCallback(std::function<void()> cb) { m_saveCb = std::move(cb); }

    /** Set FPS value for display. */
    void setFPS(float fps) { m_fps = fps; }

    /** Check if console wants keyboard input (prevents game from consuming keys). */
    bool wantsKeyboardInput() const { return m_open; }

private:
    void executeCommand(const std::string& input);
    std::vector<std::string> tokenize(const std::string& input) const;

    bool m_open = false;

    // Input field
    std::string m_inputBuffer;
    int m_cursorPos = 0;

    // Output log
    struct OutputLine {
        std::string text;
        Color color;
    };
    std::vector<std::string> m_outputText;   // Plain text for external access
    std::vector<OutputLine> m_outputLines;    // Internal with color
    float m_scrollOffset = 0.0f;
    static constexpr int MAX_OUTPUT_LINES = 200;

    // Command history
    std::vector<std::string> m_history;
    int m_historyIndex = -1;
    static constexpr int MAX_HISTORY = 50;

    // Registered commands
    struct CommandEntry {
        CommandHandler handler;
        std::string helpText;
    };
    std::unordered_map<std::string, CommandEntry> m_commands;

    // Settings store (key-value pairs modified by "set" command)
    std::unordered_map<std::string, std::string> m_settings;

    // Callbacks
    std::function<void()> m_quitCb;
    std::function<void()> m_saveCb;

    // FPS tracking
    float m_fps = 0.0f;

    // Visual constants
    static constexpr float CONSOLE_HEIGHT_FRACTION = 0.4f;  // 40% of screen height
    static constexpr float INPUT_HEIGHT = 24.0f;
    static constexpr float LINE_HEIGHT = 16.0f;
    static constexpr float PADDING = 8.0f;
    static constexpr float CHAR_WIDTH = 8.0f;  // bitmap font character width
};

} // namespace atlas
