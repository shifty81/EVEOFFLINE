#ifndef SPACE_COLORS_H
#define SPACE_COLORS_H

namespace UI {

/**
 * @file space_colors.h
 *
 * Space-themed UI color palette.
 *
 * Legacy compatibility header — prefer aatlas::Theme (atlas_types.h) for
 * new Atlas-based code.  Kept so existing ImGui-based panels continue to
 * compile while they are incrementally migrated to Atlas widgets.
 */
struct SpaceColors {
    // Background colors — deep dark blue-black
    static constexpr float BG_PRIMARY[4] = {0.051f, 0.067f, 0.090f, 0.92f};
    static constexpr float BG_SECONDARY[4] = {0.086f, 0.106f, 0.133f, 0.90f};
    static constexpr float BG_PANEL[4] = {0.031f, 0.047f, 0.071f, 0.95f};
    static constexpr float BG_HEADER[4] = {0.039f, 0.055f, 0.078f, 1.0f};
    static constexpr float BG_TOOLTIP[4] = {0.110f, 0.129f, 0.157f, 0.95f};

    // Accent colors — teal/cyan (default accent)
    static constexpr float ACCENT_PRIMARY[4] = {0.271f, 0.816f, 0.910f, 1.0f};
    static constexpr float ACCENT_SECONDARY[4] = {0.471f, 0.882f, 0.941f, 1.0f};
    static constexpr float ACCENT_DIM[4] = {0.165f, 0.353f, 0.416f, 1.0f};

    // Selection / interaction
    static constexpr float SELECTION[4] = {0.102f, 0.227f, 0.290f, 0.80f};

    // Border colors
    static constexpr float BORDER_NORMAL[4] = {0.157f, 0.220f, 0.282f, 0.6f};
    static constexpr float BORDER_HIGHLIGHT[4] = {0.271f, 0.816f, 0.910f, 0.8f};
    static constexpr float BORDER_SUBTLE[4] = {0.118f, 0.165f, 0.212f, 0.5f};

    // Text colors
    static constexpr float TEXT_PRIMARY[4] = {0.902f, 0.929f, 0.953f, 1.0f};
    static constexpr float TEXT_SECONDARY[4] = {0.545f, 0.580f, 0.620f, 1.0f};
    static constexpr float TEXT_DISABLED[4] = {0.282f, 0.310f, 0.345f, 0.6f};

    // Health colors
    static constexpr float SHIELD_COLOR[4] = {0.2f, 0.6f, 1.0f, 1.0f};
    static constexpr float ARMOR_COLOR[4] = {1.0f, 0.816f, 0.251f, 1.0f};
    static constexpr float HULL_COLOR[4] = {0.902f, 0.271f, 0.271f, 1.0f};

    // Target / standing colors
    static constexpr float TARGET_HOSTILE[4] = {0.8f, 0.2f, 0.2f, 1.0f};
    static constexpr float TARGET_FRIENDLY[4] = {0.2f, 0.6f, 1.0f, 1.0f};
    static constexpr float TARGET_NEUTRAL[4] = {0.667f, 0.667f, 0.667f, 1.0f};

    // Feedback colors
    static constexpr float SUCCESS[4] = {0.2f, 0.8f, 0.4f, 1.0f};
    static constexpr float WARNING[4] = {1.0f, 0.722f, 0.2f, 1.0f};
    static constexpr float DANGER[4] = {1.0f, 0.2f, 0.2f, 1.0f};
};

} // namespace UI

#endif // SPACE_COLORS_H
