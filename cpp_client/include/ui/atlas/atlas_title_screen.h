#pragma once

/**
 * @file atlas_title_screen.h
 * @brief Title/main menu screen for the Atlas Engine
 *
 * Shown on application startup before entering the game. Provides options to:
 *   - Play (Undock / Enter Space)
 *   - Settings (audio, graphics)
 *   - Quit
 *
 * Styled consistently with the Atlas UI Photon Dark theme, using the
 * same sidebar-inspired layout as the in-game UI.
 */

#include "atlas_context.h"
#include <string>
#include <functional>

namespace atlas {

class AtlasTitleScreen {
public:
    AtlasTitleScreen();
    ~AtlasTitleScreen() = default;

    // ── State ───────────────────────────────────────────────────────

    /** Check if the title screen is active (should be shown). */
    bool isActive() const { return m_active; }

    /** Set whether the title screen is active. */
    void setActive(bool active) { m_active = active; }

    // ── Rendering ───────────────────────────────────────────────────

    /** Render the title screen. Call between beginFrame/endFrame. */
    void render(AtlasContext& ctx);

    // ── Callbacks ───────────────────────────────────────────────────

    /** Set callback for Play button (enters the game). */
    void setPlayCallback(std::function<void()> cb) { m_playCb = std::move(cb); }

    /** Set callback for Quit button. */
    void setQuitCallback(std::function<void()> cb) { m_quitCb = std::move(cb); }

    // ── Settings access (shared with pause menu) ────────────────────

    float getMasterVolume() const { return m_masterVolume; }
    void setMasterVolume(float v) { m_masterVolume = v; }

    float getMusicVolume() const { return m_musicVolume; }
    void setMusicVolume(float v) { m_musicVolume = v; }

    float getSfxVolume() const { return m_sfxVolume; }
    void setSfxVolume(float v) { m_sfxVolume = v; }

    /** Check if the title screen wants keyboard input. */
    bool wantsKeyboardInput() const { return m_active; }

private:
    void renderMainMenu(AtlasContext& ctx);
    void renderSettings(AtlasContext& ctx);

    bool m_active = true;

    enum class Page {
        MAIN,
        SETTINGS
    };
    Page m_currentPage = Page::MAIN;

    // Audio settings
    float m_masterVolume = 0.8f;
    float m_musicVolume = 0.5f;
    float m_sfxVolume = 0.7f;

    // Callbacks
    std::function<void()> m_playCb;
    std::function<void()> m_quitCb;

    // Visual constants
    static constexpr float SIDEBAR_WIDTH = 56.0f;
    static constexpr float MENU_WIDTH = 320.0f;
    static constexpr float BUTTON_HEIGHT = 40.0f;
    static constexpr float BUTTON_SPACING = 14.0f;
    static constexpr float PADDING = 24.0f;
};

} // namespace atlas
