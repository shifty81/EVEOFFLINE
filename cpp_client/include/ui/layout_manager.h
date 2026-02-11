#pragma once

/**
 * @file layout_manager.h
 * @brief UI Layout Manager for saving, loading, and switching panel layouts
 *
 * Phase 4.10: Window Management & Customization
 *
 * Provides JSON-based serialization of panel positions, sizes, visibility,
 * and opacity, with support for named layout presets (Default, Combat,
 * Mining, Custom).
 *
 * Layout files are stored as JSON in a configurable directory (default:
 * "ui_layouts/" relative to the executable).
 */

#include "atlas/atlas_types.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace UI {

/**
 * Serialisable snapshot of a single panel's layout.
 */
struct PanelLayout {
    std::string id;
    float x = 0.0f;
    float y = 0.0f;
    float w = 300.0f;
    float h = 400.0f;
    bool  visible = true;
    bool  minimized = false;
    float opacity = 0.92f;
};

/**
 * A named collection of panel layouts.
 */
struct LayoutPreset {
    std::string name;
    std::vector<PanelLayout> panels;
};

/**
 * LayoutManager — save, load, and switch between UI layout presets.
 *
 * Each preset is stored as a JSON file in the layout directory:
 *   ui_layouts/<preset_name>.json
 *
 * Built-in preset names: "default", "combat", "mining"
 */
class LayoutManager {
public:
    LayoutManager();
    ~LayoutManager() = default;

    /**
     * Set the directory used for layout files.
     * @param dir  Path to directory (trailing slash optional).
     */
    void SetLayoutDirectory(const std::string& dir);

    /**
     * Save the current panel layout to a named preset file.
     * @param name    Preset name (becomes the filename).
     * @param panels  Current panel states keyed by panel id.
     * @return true on success.
     */
    bool SaveLayout(const std::string& name,
                    const std::unordered_map<std::string, PanelLayout>& panels);

    /**
     * Load a named preset from disk.
     * @param name   Preset name.
     * @param[out] panels  Loaded panel states.
     * @return true on success (false if file missing or corrupt).
     */
    bool LoadLayout(const std::string& name,
                    std::unordered_map<std::string, PanelLayout>& panels);

    /**
     * Get the list of available preset names (scanned from layout directory).
     */
    std::vector<std::string> GetAvailablePresets() const;

    /**
     * Delete a saved preset file.
     * @return true if deleted successfully.
     */
    bool DeletePreset(const std::string& name);

    /**
     * Create built-in default presets (default, combat, mining).
     * Only writes files that do not already exist.
     *
     * @param windowW  Window width for computing default positions.
     * @param windowH  Window height for computing default positions.
     */
    void CreateDefaultPresets(int windowW, int windowH);

    /**
     * Serialize a layout map to a JSON string (no file I/O).
     */
    static std::string SerializeToJson(
        const std::string& name,
        const std::unordered_map<std::string, PanelLayout>& panels);

    /**
     * Deserialize a JSON string into a layout map (no file I/O).
     * @return true on success.
     */
    static bool DeserializeFromJson(
        const std::string& json,
        std::string& outName,
        std::unordered_map<std::string, PanelLayout>& outPanels);

private:
    std::string m_layoutDir;

    /** Build the full file path for a preset name. */
    std::string PresetPath(const std::string& name) const;
};

} // namespace UI
