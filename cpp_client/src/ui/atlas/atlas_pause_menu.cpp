#include "ui/atlas/atlas_pause_menu.h"
#include "ui/atlas/atlas_widgets.h"

namespace atlas {

AtlasPauseMenu::AtlasPauseMenu() = default;

void AtlasPauseMenu::toggle() {
    m_open = !m_open;
    if (m_open) {
        m_currentPage = Page::MAIN;
    }
}

void AtlasPauseMenu::render(AtlasContext& ctx) {
    if (!m_open) return;

    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();
    float windowW = static_cast<float>(ctx.input().windowW);
    float windowH = static_cast<float>(ctx.input().windowH);

    // Full-screen dark overlay
    Rect overlay(0.0f, 0.0f, windowW, windowH);
    r.drawRect(overlay, Color(0.0f, 0.0f, 0.0f, 0.65f));

    // Centered panel
    float panelX = (windowW - PANEL_WIDTH) * 0.5f;
    float panelY = (windowH - PANEL_HEIGHT) * 0.5f;
    Rect panel(panelX, panelY, PANEL_WIDTH, PANEL_HEIGHT);

    // Panel background
    r.drawRect(panel, theme.bgPanel);
    // Panel border
    r.drawRect(Rect(panel.x, panel.y, panel.w, 1.0f), theme.borderNormal);
    r.drawRect(Rect(panel.x, panel.bottom() - 1.0f, panel.w, 1.0f), theme.borderNormal);
    r.drawRect(Rect(panel.x, panel.y, 1.0f, panel.h), theme.borderNormal);
    r.drawRect(Rect(panel.right() - 1.0f, panel.y, 1.0f, panel.h), theme.borderNormal);

    // Header
    Rect header(panel.x, panel.y, panel.w, theme.headerHeight + 4.0f);
    r.drawRect(header, theme.bgHeader);

    // Consume mouse over the panel
    if (ctx.isHovered(overlay)) {
        ctx.consumeMouse();
    }

    switch (m_currentPage) {
        case Page::MAIN:
            renderMainMenu(ctx, panel);
            break;
        case Page::SETTINGS:
            renderSettings(ctx, panel);
            break;
    }
}

void AtlasPauseMenu::renderMainMenu(AtlasContext& ctx, const Rect& panel) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();

    // Title
    const char* title = "PAUSED";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(panel.x + (panel.w - titleW) * 0.5f, panel.y + 6.0f),
               theme.accentSecondary);

    // Button area
    float contentY = panel.y + theme.headerHeight + 4.0f + PADDING;
    float btnW = panel.w - PADDING * 2.0f;
    float btnX = panel.x + PADDING;

    // Resume
    Rect resumeBtn(btnX, contentY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Resume", resumeBtn)) {
        m_open = false;
        if (m_resumeCb) m_resumeCb();
    }
    contentY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Settings
    Rect settingsBtn(btnX, contentY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Settings", settingsBtn)) {
        m_currentPage = Page::SETTINGS;
    }
    contentY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Save Game
    Rect saveBtn(btnX, contentY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Save Game", saveBtn)) {
        if (m_saveCb) m_saveCb();
    }
    contentY += BUTTON_HEIGHT + BUTTON_SPACING;

    // Separator
    separator(ctx, Vec2(btnX, contentY), btnW);
    contentY += BUTTON_SPACING;

    // Quit Game
    Rect quitBtn(btnX, contentY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Quit Game", quitBtn)) {
        if (m_quitCb) m_quitCb();
    }
}

void AtlasPauseMenu::renderSettings(AtlasContext& ctx, const Rect& panel) {
    const auto& theme = ctx.theme();
    auto& r = ctx.renderer();

    // Title
    const char* title = "SETTINGS";
    float titleW = r.measureText(title);
    r.drawText(title, Vec2(panel.x + (panel.w - titleW) * 0.5f, panel.y + 6.0f),
               theme.accentSecondary);

    float contentY = panel.y + theme.headerHeight + 4.0f + PADDING;
    float sliderW = panel.w - PADDING * 2.0f;
    float sliderX = panel.x + PADDING;

    // Audio section header
    label(ctx, Vec2(sliderX, contentY), "Audio", theme.accentSecondary);
    contentY += 22.0f;

    // Master Volume
    label(ctx, Vec2(sliderX, contentY), "Master Volume", theme.textSecondary);
    contentY += 18.0f;
    Rect masterSlider(sliderX, contentY, sliderW, 20.0f);
    slider(ctx, "master_vol", masterSlider, &m_masterVolume, 0.0f, 1.0f, "%.0f%%");
    contentY += 20.0f + BUTTON_SPACING;

    // Music Volume
    label(ctx, Vec2(sliderX, contentY), "Music Volume", theme.textSecondary);
    contentY += 18.0f;
    Rect musicSlider(sliderX, contentY, sliderW, 20.0f);
    slider(ctx, "music_vol", musicSlider, &m_musicVolume, 0.0f, 1.0f, "%.0f%%");
    contentY += 20.0f + BUTTON_SPACING;

    // SFX Volume
    label(ctx, Vec2(sliderX, contentY), "SFX Volume", theme.textSecondary);
    contentY += 18.0f;
    Rect sfxSlider(sliderX, contentY, sliderW, 20.0f);
    slider(ctx, "sfx_vol", sfxSlider, &m_sfxVolume, 0.0f, 1.0f, "%.0f%%");
    contentY += 20.0f + BUTTON_SPACING;

    // UI Volume
    label(ctx, Vec2(sliderX, contentY), "UI Volume", theme.textSecondary);
    contentY += 18.0f;
    Rect uiSlider(sliderX, contentY, sliderW, 20.0f);
    slider(ctx, "ui_vol", uiSlider, &m_uiVolume, 0.0f, 1.0f, "%.0f%%");
    contentY += 20.0f + BUTTON_SPACING * 2.0f;

    // Back button
    float btnW = panel.w - PADDING * 2.0f;
    Rect backBtn(sliderX, contentY, btnW, BUTTON_HEIGHT);
    if (button(ctx, "Back", backBtn)) {
        m_currentPage = Page::MAIN;
    }
}

} // namespace atlas
