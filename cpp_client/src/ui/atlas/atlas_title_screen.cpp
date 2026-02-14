#include "ui/atlas/atlas_title_screen.h"
#include "ui/atlas/atlas_widgets.h"

namespace atlas {

AtlasTitleScreen::AtlasTitleScreen() = default;

void AtlasTitleScreen::render(AtlasContext& ctx) {
    if (!m_active) return;

    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    // Full-screen background
    Rect bg(0.0f, 0.0f, windowW, windowH);
    r.drawRect(bg, Color(0.02f, 0.03f, 0.05f, 1.0f));

    // Sidebar strip (Atlas/Neocom style left edge)
    Rect sidebar(0.0f, 0.0f, SIDEBAR_WIDTH, windowH);
    r.drawRect(sidebar, theme.bgHeader);

    // Sidebar accent line
    r.drawRect(Rect(SIDEBAR_WIDTH - 1.0f, 0.0f, 1.0f, windowH),
               theme.accentSecondary.withAlpha(0.3f));

    // "A" logo in sidebar (Atlas Engine branding)
    float logoY = 16.0f;
    r.drawText("A", Vec2(SIDEBAR_WIDTH * 0.5f - 4.0f, logoY),
               theme.accentSecondary);

    // Consume mouse
    if (ctx.isHovered(bg)) {
        ctx.consumeMouse();
    }

    switch (m_currentPage) {
        case Page::MAIN:
            renderMainMenu(ctx);
            break;
        case Page::SETTINGS:
            renderSettings(ctx);
            break;
    }
}

void AtlasTitleScreen::renderMainMenu(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    // Title text — centered in the available area (right of sidebar)
    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Game title
    const char* titleLine1 = "ATLAS";
    const char* titleLine2 = "E V E   O F F L I N E";
    float title1W = r.measureText(titleLine1);
    float title2W = r.measureText(titleLine2);

    float titleY = windowH * 0.2f;
    r.drawText(titleLine1, Vec2(contentX + (contentW - title1W) * 0.5f, titleY),
               theme.accentSecondary);
    r.drawText(titleLine2, Vec2(contentX + (contentW - title2W) * 0.5f, titleY + 24.0f),
               theme.textSecondary);

    // Menu buttons — centered vertically below title
    float menuX = contentX + (contentW - MENU_WIDTH) * 0.5f;
    float menuY = windowH * 0.4f;

    // Play / Undock
    Rect playBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Undock", playBtn)) {
        m_active = false;
        if (m_playCb) m_playCb();
    }
    menuY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Settings
    Rect settingsBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Settings", settingsBtn)) {
        m_currentPage = Page::SETTINGS;
    }
    menuY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Quit
    Rect quitBtn(menuX, menuY, MENU_WIDTH, BUTTON_HEIGHT);
    if (button(ctx, "Quit", quitBtn)) {
        if (m_quitCb) m_quitCb();
    }

    // Version info at bottom
    const char* version = "Atlas Engine v1.0.0";
    float verW = r.measureText(version);
    r.drawText(version, Vec2(contentX + (contentW - verW) * 0.5f, windowH - 30.0f),
               theme.textMuted);
}

void AtlasTitleScreen::renderSettings(AtlasContext& ctx) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    float contentX = SIDEBAR_WIDTH;
    float contentW = windowW - contentX;

    // Settings title
    const char* title = "SETTINGS";
    float titleW = r.measureText(title);
    float titleY = windowH * 0.15f;
    r.drawText(title, Vec2(contentX + (contentW - titleW) * 0.5f, titleY),
               theme.accentSecondary);

    // Settings panel centered
    float panelW = 400.0f;
    float panelX = contentX + (contentW - panelW) * 0.5f;
    float panelY = windowH * 0.25f;

    // Audio section
    label(ctx, Vec2(panelX, panelY), "Audio", theme.accentSecondary);
    panelY += 24.0f;

    // Master Volume
    label(ctx, Vec2(panelX, panelY), "Master Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect masterSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_master_vol", masterSlider, &m_masterVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING;

    // Music Volume
    label(ctx, Vec2(panelX, panelY), "Music Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect musicSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_music_vol", musicSlider, &m_musicVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING;

    // SFX Volume
    label(ctx, Vec2(panelX, panelY), "SFX Volume", theme.textSecondary);
    panelY += 18.0f;
    Rect sfxSlider(panelX, panelY, panelW, 20.0f);
    slider(ctx, "title_sfx_vol", sfxSlider, &m_sfxVolume, 0.0f, 1.0f, "%.0f%%");
    panelY += 20.0f + BUTTON_SPACING * 2.0f;

    // Back button
    float btnW = 200.0f;
    float btnX = contentX + (contentW - btnW) * 0.5f;
    Rect backBtn(btnX, panelY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::MAIN;
    }
}

} // namespace atlas
