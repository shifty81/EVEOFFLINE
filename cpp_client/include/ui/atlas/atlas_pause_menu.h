#pragma once

/**
 * @file atlas_pause_menu.h
 * @brief In-game pause/escape menu for the Atlas Engine
 *
 * Opened with the ESC key. Provides access to:
 *   - Resume (close menu)
 *   - Settings (audio volume, graphics options)
 *   - Save Game
 *   - Quit to Title / Quit Game
 *
 * The menu renders a full-screen darkened overlay with a centered
 * panel using the Atlas UI widget system.
 */

#include "atlas_context.h"
#include <string>
#include <functional>

namespace atlas {

class AtlasPauseMenu {
public:
    AtlasPauseMenu();
    ~AtlasPauseMenu() = default;

    // ── Visibility ──────────────────────────────────────────────────

    /** Toggle the pause menu open/closed. */
    void toggle();

    /** Check if pause menu is currently open. */
    bool isOpen() const { return m_open; }

    /** Set pause menu open state directly. */
    void setOpen(bool open) { m_open = open; }

    // ── Rendering ───────────────────────────────────────────────────

    /** Render the pause menu overlay. Call between beginFrame/endFrame. */
    void render(AtlasContext& ctx);

    // ── Callbacks ───────────────────────────────────────────────────

    /** Set callback for Resume button. */
    void setResumeCallback(std::function<void()> cb) { m_resumeCb = std::move(cb); }

    /** Set callback for Save Game button. */
    void setSaveCallback(std::function<void()> cb) { m_saveCb = std::move(cb); }

    /** Set callback for Quit button. */
    void setQuitCallback(std::function<void()> cb) { m_quitCb = std::move(cb); }

    // ── Settings access ─────────────────────────────────────────────

    /** Get/set master volume (0.0–1.0). */
    float getMasterVolume() const { return m_masterVolume; }
    void setMasterVolume(float v) { m_masterVolume = v; }

    /** Get/set music volume (0.0–1.0). */
    float getMusicVolume() const { return m_musicVolume; }
    void setMusicVolume(float v) { m_musicVolume = v; }

    /** Get/set SFX volume (0.0–1.0). */
    float getSfxVolume() const { return m_sfxVolume; }
    void setSfxVolume(float v) { m_sfxVolume = v; }

    /** Get/set UI volume (0.0–1.0). */
    float getUiVolume() const { return m_uiVolume; }
    void setUiVolume(float v) { m_uiVolume = v; }

    /** Check if the pause menu wants keyboard input. */
    bool wantsKeyboardInput() const { return m_open; }

private:
    void renderMainMenu(AtlasContext& ctx, const Rect& panel);
    void renderSettings(AtlasContext& ctx, const Rect& panel);

    bool m_open = false;

    enum class Page {
        MAIN,
        SETTINGS
    };
    Page m_currentPage = Page::MAIN;

    // Audio settings
    float m_masterVolume = 0.8f;
    float m_musicVolume = 0.5f;
    float m_sfxVolume = 0.7f;
    float m_uiVolume = 0.6f;

    // Callbacks
    std::function<void()> m_resumeCb;
    std::function<void()> m_saveCb;
    std::function<void()> m_quitCb;

    // Visual constants
    static constexpr float PANEL_WIDTH = 360.0f;
    static constexpr float PANEL_HEIGHT = 420.0f;
    static constexpr float BUTTON_HEIGHT = 36.0f;
    static constexpr float BUTTON_SPACING = 12.0f;
    static constexpr float PADDING = 20.0f;
};

} // namespace atlas
