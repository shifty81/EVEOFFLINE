#ifndef DSCAN_PANEL_H
#define DSCAN_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

/**
 * A single result entry returned by a directional scan.
 */
struct DScanResult {
    std::string name;         // Entity name
    std::string type;         // Ship/structure type
    float distance = 0.0f;    // Distance in AU
    std::string id;           // Unique entity ID

    DScanResult() = default;
    DScanResult(const std::string& name_, const std::string& type_, float dist_, const std::string& id_ = "")
        : name(name_), type(type_), distance(dist_), id(id_) {}
};

/**
 * Directional Scanner (D-Scan) panel.
 *
 * Provides a scan angle slider (5° – 360°), range slider (0.1 AU – 14.3 AU),
 * a scan button (keyboard shortcut V), and a sortable results table.
 */
class DScanPanel {
public:
    DScanPanel();
    ~DScanPanel() = default;

    /** Render the panel contents (call between ImGui Begin/End or inside a docking callback). */
    void RenderContents();

    /** Set visibility */
    void SetVisible(bool v) { m_visible = v; }
    bool IsVisible() const { return m_visible; }

    /** Populate results (e.g. from server response). */
    void SetResults(const std::vector<DScanResult>& results);

    /** Clear results. */
    void ClearResults();

    /** Get current scan parameters. */
    float GetScanAngle() const { return m_scanAngle; }
    float GetScanRange() const { return m_scanRange; }
    const std::vector<DScanResult>& GetResults() const { return m_results; }

    /** Set scan parameters programmatically. */
    void SetScanAngle(float degrees);
    void SetScanRange(float au);

    /** Returns true when the user has clicked "Scan" since the last call (one-shot). */
    bool ConsumesScanRequest();

    /** Callback type for scan requests. */
    using ScanCallback = std::function<void(float angleDeg, float rangeAU)>;
    void SetScanCallback(ScanCallback cb) { m_scanCallback = std::move(cb); }

private:
    bool m_visible = false;
    float m_scanAngle = 360.0f;   // degrees
    float m_scanRange = 14.3f;    // AU
    bool m_scanRequested = false;

    std::vector<DScanResult> m_results;

    // Sort state
    int m_sortColumn = 2;  // default sort by distance
    bool m_sortAscending = true;

    ScanCallback m_scanCallback;

    void SortResults();
};

} // namespace UI

#endif // DSCAN_PANEL_H
