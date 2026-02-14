#include "ui/atlas/atlas_console.h"
#include <algorithm>
#include <sstream>
#include <cstdio>

// GLFW key codes (mirrored to avoid GLFW dependency in header)
#ifndef GLFW_KEY_ENTER
#define GLFW_KEY_ENTER      257
#define GLFW_KEY_BACKSPACE  259
#define GLFW_KEY_UP         265
#define GLFW_KEY_DOWN       264
#define GLFW_KEY_LEFT       263
#define GLFW_KEY_RIGHT      262
#define GLFW_KEY_HOME       268
#define GLFW_KEY_END        269
#define GLFW_KEY_ESCAPE     256
#define GLFW_KEY_DELETE     261
#define GLFW_PRESS          1
#define GLFW_REPEAT         2
#endif

namespace atlas {

AtlasConsole::AtlasConsole() {
    registerBuiltinCommands();
    print("Atlas Engine Console", Color(0.28f, 0.72f, 0.82f, 1.0f));
    print("Type 'help' for available commands.", Color(0.7f, 0.74f, 0.79f, 1.0f));
}

void AtlasConsole::toggle() {
    m_open = !m_open;
    if (m_open) {
        m_inputBuffer.clear();
        m_cursorPos = 0;
        m_historyIndex = -1;
    }
}

void AtlasConsole::handleChar(unsigned int codepoint) {
    if (!m_open) return;
    if (codepoint < 32 || codepoint > 126) return;  // printable ASCII only
    if (codepoint == '`') return;  // don't type the toggle key

    char c = static_cast<char>(codepoint);
    m_inputBuffer.insert(m_inputBuffer.begin() + m_cursorPos, c);
    m_cursorPos++;
}

void AtlasConsole::handleKey(int key, int action) {
    if (!m_open) return;
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    switch (key) {
        case GLFW_KEY_ENTER:
            if (!m_inputBuffer.empty()) {
                // Echo command
                print("> " + m_inputBuffer, Color(0.92f, 0.94f, 0.96f, 1.0f));
                // Add to history
                if (m_history.empty() || m_history.back() != m_inputBuffer) {
                    m_history.push_back(m_inputBuffer);
                    if (static_cast<int>(m_history.size()) > MAX_HISTORY) {
                        m_history.erase(m_history.begin());
                    }
                }
                executeCommand(m_inputBuffer);
                m_inputBuffer.clear();
                m_cursorPos = 0;
                m_historyIndex = -1;
                // Auto-scroll to bottom
                m_scrollOffset = 0.0f;
            }
            break;

        case GLFW_KEY_BACKSPACE:
            if (m_cursorPos > 0) {
                m_inputBuffer.erase(m_inputBuffer.begin() + m_cursorPos - 1);
                m_cursorPos--;
            }
            break;

        case GLFW_KEY_DELETE:
            if (m_cursorPos < static_cast<int>(m_inputBuffer.size())) {
                m_inputBuffer.erase(m_inputBuffer.begin() + m_cursorPos);
            }
            break;

        case GLFW_KEY_LEFT:
            if (m_cursorPos > 0) m_cursorPos--;
            break;

        case GLFW_KEY_RIGHT:
            if (m_cursorPos < static_cast<int>(m_inputBuffer.size())) m_cursorPos++;
            break;

        case GLFW_KEY_HOME:
            m_cursorPos = 0;
            break;

        case GLFW_KEY_END:
            m_cursorPos = static_cast<int>(m_inputBuffer.size());
            break;

        case GLFW_KEY_UP:
            if (!m_history.empty()) {
                if (m_historyIndex < 0) {
                    m_historyIndex = static_cast<int>(m_history.size()) - 1;
                } else if (m_historyIndex > 0) {
                    m_historyIndex--;
                }
                m_inputBuffer = m_history[m_historyIndex];
                m_cursorPos = static_cast<int>(m_inputBuffer.size());
            }
            break;

        case GLFW_KEY_DOWN:
            if (m_historyIndex >= 0) {
                m_historyIndex++;
                if (m_historyIndex >= static_cast<int>(m_history.size())) {
                    m_historyIndex = -1;
                    m_inputBuffer.clear();
                } else {
                    m_inputBuffer = m_history[m_historyIndex];
                }
                m_cursorPos = static_cast<int>(m_inputBuffer.size());
            }
            break;

        case GLFW_KEY_ESCAPE:
            m_open = false;
            break;
    }
}

void AtlasConsole::render(AtlasContext& ctx) {
    if (!m_open) return;

    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float consoleH = static_cast<float>(ctx.input().windowH) * CONSOLE_HEIGHT_FRACTION;

    // Background panel
    Rect bg(0.0f, 0.0f, windowW, consoleH);
    r.drawRect(bg, Color(0.02f, 0.03f, 0.05f, 0.92f));

    // Bottom border
    r.drawRect(Rect(0.0f, consoleH - 2.0f, windowW, 2.0f), theme.accentSecondary.withAlpha(0.6f));

    // Input field background
    float inputY = consoleH - INPUT_HEIGHT - PADDING;
    Rect inputBg(PADDING, inputY, windowW - PADDING * 2.0f, INPUT_HEIGHT);
    r.drawRect(inputBg, Color(0.06f, 0.08f, 0.11f, 0.9f));
    r.drawRect(Rect(inputBg.x, inputBg.y, inputBg.w, 1.0f), theme.borderNormal);

    // Input prompt and text
    std::string prompt = "> " + m_inputBuffer;
    r.drawText(prompt, Vec2(PADDING + 4.0f, inputY + 4.0f), theme.textPrimary);

    // Cursor blink (simple steady cursor)
    float cursorX = PADDING + 4.0f + static_cast<float>(2 + m_cursorPos) * CHAR_WIDTH;
    r.drawRect(Rect(cursorX, inputY + 3.0f, 1.0f, INPUT_HEIGHT - 6.0f),
               theme.accentSecondary);

    // Output log area
    float logTop = PADDING;
    float logBottom = inputY - PADDING;
    float logHeight = logBottom - logTop;

    r.pushClip(Rect(0.0f, logTop, windowW, logHeight));

    int maxVisible = static_cast<int>(logHeight / LINE_HEIGHT);
    int totalLines = static_cast<int>(m_outputLines.size());
    int startLine = std::max(0, totalLines - maxVisible - static_cast<int>(m_scrollOffset));
    int endLine = std::min(totalLines, startLine + maxVisible + 1);

    float y = logTop;
    for (int i = startLine; i < endLine; i++) {
        r.drawText(m_outputLines[i].text, Vec2(PADDING + 4.0f, y),
                   m_outputLines[i].color);
        y += LINE_HEIGHT;
    }

    r.popClip();

    // Handle scroll input when console is open
    if (ctx.input().scrollY != 0.0f) {
        m_scrollOffset += ctx.input().scrollY * 3.0f;
        float maxScroll = std::max(0.0f, static_cast<float>(totalLines - maxVisible));
        m_scrollOffset = std::clamp(m_scrollOffset, 0.0f, maxScroll);
    }

    // Title bar
    r.drawText("ATLAS CONSOLE", Vec2(PADDING, 2.0f),
               theme.accentSecondary.withAlpha(0.5f));

    // Consume mouse so game world doesn't respond to clicks on the console
    if (ctx.isHovered(bg)) {
        ctx.consumeMouse();
    }
}

void AtlasConsole::registerCommand(const std::string& name, CommandHandler handler,
                                    const std::string& helpText) {
    m_commands[name] = {std::move(handler), helpText};
}

void AtlasConsole::print(const std::string& text) {
    print(text, Color(0.92f, 0.94f, 0.96f, 1.0f));  // default: textPrimary
}

void AtlasConsole::print(const std::string& text, const Color& color) {
    m_outputText.push_back(text);
    m_outputLines.push_back({text, color});
    if (static_cast<int>(m_outputLines.size()) > MAX_OUTPUT_LINES) {
        m_outputText.erase(m_outputText.begin());
        m_outputLines.erase(m_outputLines.begin());
    }
}

void AtlasConsole::clearOutput() {
    m_outputText.clear();
    m_outputLines.clear();
    m_scrollOffset = 0.0f;
}

void AtlasConsole::registerBuiltinCommands() {
    // help — list all commands
    registerCommand("help", [this](const std::vector<std::string>&) {
        print("Available commands:", Color(0.28f, 0.72f, 0.82f, 1.0f));
        for (const auto& pair : m_commands) {
            std::string line = "  " + pair.first;
            if (!pair.second.helpText.empty()) {
                line += " - " + pair.second.helpText;
            }
            print(line, Color(0.7f, 0.74f, 0.79f, 1.0f));
        }
    }, "Show this help message");

    // clear — clear console output
    registerCommand("clear", [this](const std::vector<std::string>&) {
        clearOutput();
    }, "Clear the console output");

    // quit — quit the game
    registerCommand("quit", [this](const std::vector<std::string>&) {
        print("Shutting down...", Color(0.92f, 0.68f, 0.22f, 1.0f));
        if (m_quitCb) m_quitCb();
    }, "Quit the game");

    // exit — alias for quit
    registerCommand("exit", [this](const std::vector<std::string>&) {
        print("Shutting down...", Color(0.92f, 0.68f, 0.22f, 1.0f));
        if (m_quitCb) m_quitCb();
    }, "Quit the game (alias for quit)");

    // fps — show current FPS
    registerCommand("fps", [this](const std::vector<std::string>&) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "FPS: %.1f (%.2f ms/frame)", m_fps, m_fps > 0.0f ? 1000.0f / m_fps : 0.0f);
        print(buf, Color(0.2f, 0.8f, 0.4f, 1.0f));
    }, "Show current frame rate");

    // save — force save game state
    registerCommand("save", [this](const std::vector<std::string>&) {
        print("Saving game state...", Color(0.92f, 0.68f, 0.22f, 1.0f));
        if (m_saveCb) {
            m_saveCb();
            print("Game saved.", Color(0.2f, 0.8f, 0.4f, 1.0f));
        } else {
            print("Save not available (no save handler registered).", Color(0.86f, 0.26f, 0.26f, 1.0f));
        }
    }, "Force save game state");

    // set — modify a setting (key=value)
    registerCommand("set", [this](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            // List all settings
            if (m_settings.empty()) {
                print("No settings configured. Usage: set <key> <value>",
                      Color(0.7f, 0.74f, 0.79f, 1.0f));
            } else {
                print("Current settings:", Color(0.28f, 0.72f, 0.82f, 1.0f));
                for (const auto& pair : m_settings) {
                    print("  " + pair.first + " = " + pair.second,
                          Color(0.7f, 0.74f, 0.79f, 1.0f));
                }
            }
            return;
        }
        if (args.size() < 3) {
            // Show current value
            auto it = m_settings.find(args[1]);
            if (it != m_settings.end()) {
                print(args[1] + " = " + it->second, Color(0.7f, 0.74f, 0.79f, 1.0f));
            } else {
                print("Setting '" + args[1] + "' not found. Usage: set <key> <value>",
                      Color(0.92f, 0.68f, 0.22f, 1.0f));
            }
            return;
        }
        m_settings[args[1]] = args[2];
        print("Set " + args[1] + " = " + args[2], Color(0.2f, 0.8f, 0.4f, 1.0f));
    }, "Get/set a setting: set [key] [value]");

    // echo — print text
    registerCommand("echo", [this](const std::vector<std::string>& args) {
        std::string msg;
        for (size_t i = 1; i < args.size(); i++) {
            if (i > 1) msg += " ";
            msg += args[i];
        }
        print(msg);
    }, "Print text to console");
}

void AtlasConsole::executeCommand(const std::string& input) {
    auto tokens = tokenize(input);
    if (tokens.empty()) return;

    std::string cmd = tokens[0];
    // Convert to lowercase for case-insensitive matching
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    auto it = m_commands.find(cmd);
    if (it != m_commands.end()) {
        it->second.handler(tokens);
    } else {
        print("Unknown command: " + cmd + ". Type 'help' for available commands.",
              Color(0.86f, 0.26f, 0.26f, 1.0f));
    }
}

std::vector<std::string> AtlasConsole::tokenize(const std::string& input) const {
    std::vector<std::string> tokens;
    std::istringstream stream(input);
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

} // namespace atlas
