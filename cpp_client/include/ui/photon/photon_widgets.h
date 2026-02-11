#pragma once

/**
 * @file photon_widgets.h
 * @brief High-level Photon UI widgets modelled after EVE Online's Photon UI
 *
 * All widgets are free functions that take a PhotonContext& and draw
 * directly through its renderer.  This mirrors an immediate-mode API
 * but the renderer batches everything for a single GPU draw call.
 *
 * Widget gallery (based on the 3drenderon.png reference screenshot):
 *
 *   Panel       — dark translucent rectangle with optional header bar,
 *                 close/minimize buttons, and border.  Sharp corners.
 *   Button      — small rectangular button, highlight on hover.
 *   ProgressBar — horizontal bar (shield/armor/hull/capacitor bars).
 *   StatusArc   — concentric semicircular arcs for ship HP display.
 *   CapRing     — segmented circular capacitor gauge.
 *   ModuleSlot  — circular icon button for fitted modules.
 *   OverviewRow — single row in the overview table.
 *   TargetCard  — locked-target thumbnail (top-center row).
 *   Label       — simple text label with optional color.
 *   Separator   — thin horizontal rule.
 *   TreeNode    — collapsible tree entry (People & Places style).
 *   Scrollbar   — thin vertical scroll indicator.
 */

#include "photon_context.h"
#include <string>
#include <vector>
#include <functional>

namespace photon {

// ── Panel ───────────────────────────────────────────────────────────

struct PanelFlags {
    bool showHeader    = true;   // dark header bar with title text
    bool showClose     = true;   // × button in header
    bool showMinimize  = true;   // — button in header
    bool compactMode   = false;  // reduced padding (EVE compact mode)
    bool locked        = false;  // prevent drag/resize
    bool drawBorder    = true;   // subtle border around panel
};

/**
 * Begin a Photon panel.  Returns true if the panel is open (not
 * minimized).  Call panelEnd() when done adding content.
 *
 * @param ctx      Photon context.
 * @param title    Header title text.
 * @param bounds   Position and size in screen pixels.
 * @param flags    Visual/behavioral flags.
 * @param open     If non-null, the × button writes false here.
 */
bool panelBegin(PhotonContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags = {},
                bool* open = nullptr);

/** End the current panel. */
void panelEnd(PhotonContext& ctx);

// ── Buttons ─────────────────────────────────────────────────────────

/** Rectangular text button.  Returns true on click. */
bool button(PhotonContext& ctx, const char* label, const Rect& r);

/** Small icon-style square button (Neocom style). */
bool iconButton(PhotonContext& ctx, WidgetID id, const Rect& r,
                const Color& iconColor);

// ── Progress / Status Bars ──────────────────────────────────────────

/**
 * Horizontal progress bar with label (e.g. "Shield: 89%").
 * Draws background + filled portion + optional percentage text.
 */
void progressBar(PhotonContext& ctx, const Rect& r,
                 float fraction, const Color& fillColor,
                 const char* label = nullptr);

// ── Ship HUD Widgets ────────────────────────────────────────────────

/**
 * Draw the three concentric shield/armor/hull semicircle arcs.
 *
 * Layout (from screenshot): arcs sweep the TOP half of a circle,
 * with shield outermost, hull innermost.  Percentage labels sit
 * to the left of each arc.
 *
 * @param centre    Centre point of the HUD circle.
 * @param outerR    Outer radius of the shield arc.
 * @param shieldPct 0.0–1.0 shield remaining.
 * @param armorPct  0.0–1.0 armor remaining.
 * @param hullPct   0.0–1.0 hull remaining.
 */
void shipStatusArcs(PhotonContext& ctx, Vec2 centre, float outerR,
                    float shieldPct, float armorPct, float hullPct);

/**
 * Draw a segmented capacitor ring around the HUD centre.
 *
 * The ring is divided into N segments (typically 10–20 depending
 * on ship).  Filled segments are bright teal, depleted are dark.
 *
 * @param centre    Centre point.
 * @param innerR    Inner radius of the ring.
 * @param outerR    Outer radius of the ring.
 * @param fraction  0.0–1.0 capacitor remaining.
 * @param segments  Number of segments (default 16).
 */
void capacitorRing(PhotonContext& ctx, Vec2 centre,
                   float innerR, float outerR,
                   float fraction, int segments = 16);

/**
 * Draw a single circular module slot button.
 *
 * @param centre     Centre of the circle.
 * @param radius     Circle radius (~20-24px).
 * @param active     Whether the module is currently cycling.
 * @param cooldownPct 0.0–1.0 cooldown remaining (for sweep overlay).
 * @param color      Module highlight color.
 * @return true if clicked.
 */
bool moduleSlot(PhotonContext& ctx, Vec2 centre, float radius,
                bool active, float cooldownPct, const Color& color);

/**
 * Draw a module slot with overheat indication.
 *
 * @param centre      Centre of the circle.
 * @param radius      Circle radius (~20-24px).
 * @param active      Whether the module is currently cycling.
 * @param cooldownPct 0.0–1.0 cooldown remaining.
 * @param color       Module highlight color.
 * @param overheatPct 0.0–1.0 heat damage level. At 1.0 the module is burnt out.
 * @param time        Current game time in seconds (for pulse animation).
 * @return true if clicked.
 */
bool moduleSlotEx(PhotonContext& ctx, Vec2 centre, float radius,
                  bool active, float cooldownPct, const Color& color,
                  float overheatPct, float time);

/**
 * Capacitor ring with smooth easing between values.
 *
 * Animates the displayed capacitor level toward the target fraction
 * using exponential easing, giving a smooth drain/recharge effect.
 *
 * @param ctx          Context.
 * @param centre       Centre point.
 * @param innerR       Inner radius.
 * @param outerR       Outer radius.
 * @param targetFrac   Target capacitor fraction 0.0–1.0.
 * @param displayFrac  In/out displayed fraction (lerped toward target each frame).
 * @param dt           Frame delta time in seconds.
 * @param segments     Number of ring segments.
 * @param lerpSpeed    Easing speed (higher = faster, default 5.0).
 */
void capacitorRingAnimated(PhotonContext& ctx, Vec2 centre,
                           float innerR, float outerR,
                           float targetFrac, float& displayFrac,
                           float dt, int segments = 16,
                           float lerpSpeed = 5.0f);

/**
 * Speed indicator (bottom of HUD): shows current speed with +/- buttons.
 */
void speedIndicator(PhotonContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed);

// ── Overview Widgets ────────────────────────────────────────────────

struct OverviewEntry {
    std::string name;
    std::string type;
    float distance = 0.0f;
    float velocity = 0.0f;
    Color standingColor;      // red/blue/grey for hostile/friendly/neutral
    bool  selected = false;
};

/**
 * Draw the overview table header (columns: Distance, Name, Type, Velocity).
 */
void overviewHeader(PhotonContext& ctx, const Rect& r,
                    const std::vector<std::string>& tabs,
                    int activeTab);

/**
 * Draw a single overview row.  Returns true if clicked.
 */
bool overviewRow(PhotonContext& ctx, const Rect& r,
                 const OverviewEntry& entry, bool isAlternate);

// ── Locked Target Cards ─────────────────────────────────────────────

struct TargetCardInfo {
    std::string name;
    float shieldPct = 1.0f;
    float armorPct  = 1.0f;
    float hullPct   = 1.0f;
    float distance  = 0.0f;
    bool  isHostile = false;
    bool  isActive  = false;   // currently selected target
};

/**
 * Draw a locked-target card (the small thumbnail shown in the
 * top-center row).  Returns true if clicked.
 *
 * @param r      Rectangle for this card (~80×80 px).
 * @param info   Target data.
 */
bool targetCard(PhotonContext& ctx, const Rect& r,
                const TargetCardInfo& info);

// ── Selected Item Panel ─────────────────────────────────────────────

struct SelectedItemInfo {
    std::string name;
    float distance = 0.0f;
    std::string distanceUnit = "km";
};

/**
 * Draw the "Selected Item" panel (top-right corner) showing
 * the currently selected entity's name, distance, and action buttons.
 */
void selectedItemPanel(PhotonContext& ctx, const Rect& r,
                       const SelectedItemInfo& info);

// ── Utility Widgets ─────────────────────────────────────────────────

/** Simple left-aligned text label. */
void label(PhotonContext& ctx, Vec2 pos, const std::string& text,
           const Color& color = {});

/** Thin horizontal separator line. */
void separator(PhotonContext& ctx, Vec2 start, float width);

/**
 * Collapsible tree node (People & Places style).
 * Returns true if expanded.
 */
bool treeNode(PhotonContext& ctx, const Rect& r,
              const char* label, bool* expanded);

/** Thin vertical scrollbar indicator. */
void scrollbar(PhotonContext& ctx, const Rect& track,
               float scrollOffset, float contentHeight, float viewHeight);

// ── Neocom Bar ──────────────────────────────────────────────────────

/**
 * Draw the Neocom sidebar (left edge, full height).
 *
 * @param ctx       Context.
 * @param x         Left edge X position (usually 0).
 * @param width     Bar width (15px collapsed, ~56px normal).
 * @param height    Window height.
 * @param icons     Number of icon slots.
 * @param callback  Called with icon index when an icon is clicked.
 */
void neocomBar(PhotonContext& ctx, float x, float width, float height,
               int icons, const std::function<void(int)>& callback);

// ── Tooltip ─────────────────────────────────────────────────────────

/**
 * Draw a floating tooltip near the mouse cursor.
 * Call after the widget you want to annotate, only when hovered.
 *
 * @param ctx   Context.
 * @param text  Tooltip text string.
 */
void tooltip(PhotonContext& ctx, const std::string& text);

// ── Checkbox ────────────────────────────────────────────────────────

/**
 * Checkbox widget with label text.  Toggles *checked on click.
 * Returns true when the value changes.
 *
 * @param ctx      Context.
 * @param label    Text label shown to the right of the box.
 * @param r        Bounding rectangle (box is left-aligned).
 * @param checked  Pointer to bool toggled on click.
 */
bool checkbox(PhotonContext& ctx, const char* label,
              const Rect& r, bool* checked);

// ── ComboBox (dropdown selector) ────────────────────────────────────

/**
 * Dropdown combo box.  Shows the currently selected item and returns
 * true when a new selection is made.
 *
 * @param ctx         Context.
 * @param label       Widget label (for ID hashing).
 * @param r           Bounding rectangle for the collapsed combo.
 * @param items       List of option strings.
 * @param selected    Index of the currently selected item (updated on click).
 * @param dropdownOpen  Pointer to bool tracking open/closed state.
 */
bool comboBox(PhotonContext& ctx, const char* label,
              const Rect& r, const std::vector<std::string>& items,
              int* selected, bool* dropdownOpen);

// ── Stateful Panel (with drag + minimize) ───────────────────────────

/**
 * Begin a panel using persistent state (supports drag-to-move and
 * minimize/collapse).  Returns true if content area is visible.
 *
 * @param ctx    Context.
 * @param title  Header title.
 * @param state  Persistent panel state (position, open, minimized).
 * @param flags  Visual flags.
 */
bool panelBeginStateful(PhotonContext& ctx, const char* title,
                        PanelState& state, const PanelFlags& flags = {});

// ── Slider ──────────────────────────────────────────────────────────

/**
 * Horizontal slider widget.  Returns true when the value changes.
 *
 * @param ctx      Context.
 * @param label    Widget label (for ID hashing).
 * @param r        Bounding rectangle for the slider track.
 * @param value    Pointer to the current value (updated on drag).
 * @param minVal   Minimum value.
 * @param maxVal   Maximum value.
 * @param format   Printf-style format string for the value label (e.g. "%.0f°").
 *                 Pass nullptr to suppress the label.
 */
bool slider(PhotonContext& ctx, const char* label,
            const Rect& r, float* value,
            float minVal, float maxVal,
            const char* format = nullptr);

// ── Text Input ──────────────────────────────────────────────────────

/**
 * Persistent state for a text input field.
 */
struct TextInputState {
    std::string text;
    int cursorPos = 0;
    bool focused = false;
};

/**
 * Single-line text input field.  Returns true when text changes.
 *
 * @param ctx      Context.
 * @param label    Widget label (for ID hashing).
 * @param r        Bounding rectangle.
 * @param state    Persistent input state (text buffer, cursor, focus).
 * @param placeholder  Placeholder text shown when empty and unfocused.
 */
bool textInput(PhotonContext& ctx, const char* label,
               const Rect& r, TextInputState& state,
               const char* placeholder = nullptr);

// ── Notification Toast ──────────────────────────────────────────────

/**
 * Show a transient notification banner.  Call each frame while
 * the notification should be visible.
 *
 * @param ctx      Context.
 * @param text     Notification message.
 * @param color    Accent color for the left border (default: accentPrimary).
 */
void notification(PhotonContext& ctx, const std::string& text,
                  const Color& color = {});

} // namespace photon
