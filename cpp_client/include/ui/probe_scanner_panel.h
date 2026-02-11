#ifndef PROBE_SCANNER_PANEL_H
#define PROBE_SCANNER_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

struct ProbeScanResult;

/**
 * Probe Scanner panel — EVE-style exploration probe scanning UI.
 *
 * Shows a list of scan results with signal strength, provides scan
 * controls (probe deployment, scan button), and result filtering.
 */
class ProbeScannerPanel {
public:
    ProbeScannerPanel();
    ~ProbeScannerPanel() = default;

    /** Render the panel contents (call inside docking callback). */
    void RenderContents();

    /** Visibility. */
    void SetVisible(bool v) { m_visible = v; }
    bool IsVisible() const { return m_visible; }

    /** Populate results (e.g. from server response). */
    void SetResults(const std::vector<ProbeScanResult>& results);

    /** Clear all results. */
    void ClearResults();

    /** Returns true if user has clicked Scan since last call (one-shot). */
    bool ConsumesScanRequest();

    /** Set callback for scan requests. */
    using ScanCallback = std::function<void()>;
    void SetScanCallback(ScanCallback cb) { m_scanCallback = std::move(cb); }

    /** Get/Set probe count. */
    int GetProbeCount() const { return m_probeCount; }
    void SetProbeCount(int count);

    /** Get/Set probe range. */
    float GetProbeRange() const { return m_probeRange; }
    void SetProbeRange(float au);

private:
    bool m_visible = false;
    int m_probeCount = 8;      // Number of probes deployed
    float m_probeRange = 8.0f; // Probe scan range in AU

    bool m_scanRequested = false;
    std::vector<ProbeScanResult> m_results;

    // Filter state
    bool m_filterAnomalies = true;
    bool m_filterSignatures = true;
    bool m_filterShips = true;

    // Sort state
    int m_sortColumn = 4;     // default sort by signal strength
    bool m_sortAscending = false;

    ScanCallback m_scanCallback;

    void SortResults();
};

} // namespace UI

#endif // PROBE_SCANNER_PANEL_H
