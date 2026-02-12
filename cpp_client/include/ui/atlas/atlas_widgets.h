#pragma once

/**
 * @file atlas_widgets.h
 * @brief High-level Atlas UI widgets modelled after EVE Online's Atlas UI
 *
 * All widgets are free functions that take a AtlasContext& and draw
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

#include "atlas_context.h"
#include <string>
#include <vector>
#include <functional>

namespace atlas {

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
 * Begin a Atlas panel.  Returns true if the panel is open (not
 * minimized).  Call panelEnd() when done adding content.
 *
 * @param ctx      Atlas context.
 * @param title    Header title text.
 * @param bounds   Position and size in screen pixels.
 * @param flags    Visual/behavioral flags.
 * @param open     If non-null, the × button writes false here.
 */
bool panelBegin(AtlasContext& ctx, const char* title,
                Rect& bounds, const PanelFlags& flags = {},
                bool* open = nullptr);

/** End the current panel. */
void panelEnd(AtlasContext& ctx);

// ── Buttons ─────────────────────────────────────────────────────────

/** Rectangular text button.  Returns true on click. */
bool button(AtlasContext& ctx, const char* label, const Rect& r);

/** Small icon-style square button (Neocom style). */
bool iconButton(AtlasContext& ctx, WidgetID id, const Rect& r,
                const Color& iconColor, const char* symbol = nullptr);

// ── Progress / Status Bars ──────────────────────────────────────────

/**
 * Horizontal progress bar with label (e.g. "Shield: 89%").
 * Draws background + filled portion + optional percentage text.
 */
void progressBar(AtlasContext& ctx, const Rect& r,
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
void shipStatusArcs(AtlasContext& ctx, Vec2 centre, float outerR,
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
void capacitorRing(AtlasContext& ctx, Vec2 centre,
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
bool moduleSlot(AtlasContext& ctx, Vec2 centre, float radius,
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
bool moduleSlotEx(AtlasContext& ctx, Vec2 centre, float radius,
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
void capacitorRingAnimated(AtlasContext& ctx, Vec2 centre,
                           float innerR, float outerR,
                           float targetFrac, float& displayFrac,
                           float dt, int segments = 16,
                           float lerpSpeed = 5.0f);

/**
 * Speed indicator (bottom of HUD): shows current speed with +/- buttons.
 */
void speedIndicator(AtlasContext& ctx, Vec2 pos,
                    float currentSpeed, float maxSpeed);

// ── Overview Widgets ────────────────────────────────────────────────

struct OverviewEntry {
    std::string entityId;     // unique entity identifier (for selection/targeting)
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
void overviewHeader(AtlasContext& ctx, const Rect& r,
                    const std::vector<std::string>& tabs,
                    int activeTab);

/**
 * Draw a single overview row.  Returns true if clicked.
 */
bool overviewRow(AtlasContext& ctx, const Rect& r,
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
bool targetCard(AtlasContext& ctx, const Rect& r,
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
void selectedItemPanel(AtlasContext& ctx, const Rect& r,
                       const SelectedItemInfo& info);

// ── Utility Widgets ─────────────────────────────────────────────────

/** Simple left-aligned text label. */
void label(AtlasContext& ctx, Vec2 pos, const std::string& text,
           const Color& color = {});

/** Thin horizontal separator line. */
void separator(AtlasContext& ctx, Vec2 start, float width);

/**
 * Collapsible tree node (People & Places style).
 * Returns true if expanded.
 */
bool treeNode(AtlasContext& ctx, const Rect& r,
              const char* label, bool* expanded);

/** Thin vertical scrollbar indicator. */
void scrollbar(AtlasContext& ctx, const Rect& track,
               float scrollOffset, float contentHeight, float viewHeight);

// ── Sidebar Bar ─────────────────────────────────────────────────────

/**
 * Draw the sidebar (left edge, full height).
 *
 * Modelled after EVE Online's Neocom bar — a dark, minimalist,
 * semi-transparent vertical bar with:
 *   - "A" (Atlas) menu icon at the top
 *   - Character portrait area
 *   - Skill queue progress bar
 *   - Service icon slots with active-state highlighting
 *
 * @param ctx           Context.
 * @param x             Left edge X position (usually 0).
 * @param width         Bar width (~40-56px).
 * @param height        Window height.
 * @param icons         Number of icon slots.
 * @param callback      Called with icon index when an icon is clicked.
 * @param activeIcons   Optional array of bools (size >= icons) indicating
 *                      which icons have their panel currently open.
 *                      Active icons show a persistent highlight.
 * @param skillQueuePct Skill queue progress (0.0–1.0). Shown as a thin
 *                      bar below the character portrait area.
 */
void sidebarBar(AtlasContext& ctx, float x, float width, float height,
                int icons, const std::function<void(int)>& callback,
                const bool* activeIcons = nullptr,
                float skillQueuePct = 0.0f);

// ── Tooltip ─────────────────────────────────────────────────────────

/**
 * Draw a floating tooltip near the mouse cursor.
 * Call after the widget you want to annotate, only when hovered.
 *
 * @param ctx   Context.
 * @param text  Tooltip text string.
 */
void tooltip(AtlasContext& ctx, const std::string& text);

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
bool checkbox(AtlasContext& ctx, const char* label,
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
bool comboBox(AtlasContext& ctx, const char* label,
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
bool panelBeginStateful(AtlasContext& ctx, const char* title,
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
bool slider(AtlasContext& ctx, const char* label,
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
bool textInput(AtlasContext& ctx, const char* label,
               const Rect& r, TextInputState& state,
               const char* placeholder = nullptr);

// ── Mode Indicator ──────────────────────────────────────────────────

/**
 * Movement mode indicator — EVE-style on-screen text showing the
 * currently active movement mode (Approach, Orbit, Keep at Range,
 * Dock) near the ship HUD.
 *
 * @param ctx       Context.
 * @param pos       Centre position for the indicator.
 * @param modeText  Text to display (e.g. "APPROACH — click a target").
 *                  Pass empty string or nullptr to hide.
 * @param color     Accent color for the indicator.
 */
void modeIndicator(AtlasContext& ctx, Vec2 pos,
                   const char* modeText, const Color& color = {});

// ── Notification Toast ──────────────────────────────────────────────

/**
 * Show a transient notification banner.  Call each frame while
 * the notification should be visible.
 *
 * @param ctx      Context.
 * @param text     Notification message.
 * @param color    Accent color for the left border (default: accentPrimary).
 */
void notification(AtlasContext& ctx, const std::string& text,
                  const Color& color = {});

// ── Info Panel ──────────────────────────────────────────────────────

/**
 * Data for the entity info panel (Show Info).
 */
struct InfoPanelData {
    std::string name;
    std::string type;           // e.g. "Frigate", "Cruiser", "Station"
    std::string faction;        // e.g. "Crimson Order", "Venom Syndicate"
    float shieldPct   = 0.0f;   // 0–1
    float armorPct    = 0.0f;
    float hullPct     = 0.0f;
    float distance    = 0.0f;   // metres
    float velocity    = 0.0f;   // m/s
    float signature   = 0.0f;   // signature radius in metres
    bool  hasHealth   = false;  // whether to show health bars

    bool isEmpty() const { return name.empty(); }
};

/**
 * Draw an entity info panel (the "Show Info" window).
 *
 * @param ctx    Context.
 * @param state  Persistent panel state (position, open, minimized).
 * @param data   Entity information to display.
 */
void infoPanelDraw(AtlasContext& ctx, PanelState& state,
                   const InfoPanelData& data);

// ── Overview Header with click support ──────────────────────────────

/**
 * Draw overview tab header with interactive tab switching.
 * Returns the index of the clicked tab, or -1 if none clicked.
 */
int overviewHeaderInteractive(AtlasContext& ctx, const Rect& r,
                              const std::vector<std::string>& tabs,
                              int activeTab);



// ── Tab Bar (reusable tabbed header) ────────────────────────────────

/**
 * Draw a generic tab bar.  Returns the index of the clicked tab,
 * or -1 if none was clicked this frame.
 *
 * @param ctx        Context.
 * @param r          Bounding rectangle for the tab bar.
 * @param labels     Tab label strings.
 * @param activeIdx  Currently selected tab index.
 */
int tabBar(AtlasContext& ctx, const Rect& r,
           const std::vector<std::string>& labels, int activeIdx);

// ── Combat Log Widget ───────────────────────────────────────────────

/**
 * Draw a scrollable combat log message list.
 *
 * Messages are drawn newest-at-bottom in a dark panel with a thin
 * accent border.  Older messages scroll up and fade out.
 *
 * @param ctx        Context.
 * @param r          Bounding rectangle.
 * @param messages   List of log strings (newest at end).
 * @param scrollOff  Scroll offset (updated on mouse wheel).
 * @param maxVisible Maximum visible rows (0 = compute from height).
 */
void combatLogWidget(AtlasContext& ctx, const Rect& r,
                     const std::vector<std::string>& messages,
                     float& scrollOff, int maxVisible = 0);

// ── Damage Flash Overlay ────────────────────────────────────────────

/**
 * Draw a screen-edge damage flash overlay.
 *
 * Renders a coloured translucent vignette around the ship HUD centre
 * that fades over time.  Colour depends on the layer hit:
 *   Shield -> blue, Armor -> gold, Hull -> red.
 *
 * @param ctx       Context.
 * @param centre    Centre of the HUD (flash radiates outward).
 * @param radius    Outer radius of the flash effect.
 * @param layer     0 = shield, 1 = armor, 2 = hull.
 * @param intensity Current intensity (0-1, fading toward 0).
 */
void damageFlashOverlay(AtlasContext& ctx, Vec2 centre, float radius,
                        int layer, float intensity);

// ── Drone Status Bar ────────────────────────────────────────────────

/**
 * Draw a compact drone status summary widget showing deployed/bay
 * counts and bandwidth usage.
 *
 * @param ctx           Context.
 * @param r             Bounding rectangle.
 * @param inSpace       Number of drones in space.
 * @param inBay         Number of drones in bay.
 * @param bandwidthUsed Current bandwidth used (Mbit/s).
 * @param bandwidthMax  Maximum bandwidth capacity.
 */
void droneStatusBar(AtlasContext& ctx, const Rect& r,
                    int inSpace, int inBay,
                    int bandwidthUsed, int bandwidthMax);

// ── Fleet Broadcast Banner ──────────────────────────────────────────

/**
 * Data for a single fleet broadcast.
 */
struct FleetBroadcast {
    std::string sender;
    std::string message;        // e.g. "Need Armor", "Align To", "Warp To"
    Color       color;          // broadcast type accent color
    float       age = 0.0f;     // seconds since broadcast (for fade)
    float       maxAge = 8.0f;  // total display time
};

/**
 * Draw a fleet broadcast banner above the target cards.
 *
 * Shows the most recent broadcast with sender, message, and a fade
 * indicator.  Older broadcasts drop off automatically.
 *
 * @param ctx         Context.
 * @param r           Bounding rectangle.
 * @param broadcasts  Active broadcasts (newest last).
 */
void fleetBroadcastBanner(AtlasContext& ctx, const Rect& r,
                          const std::vector<FleetBroadcast>& broadcasts);

} // namespace atlas
