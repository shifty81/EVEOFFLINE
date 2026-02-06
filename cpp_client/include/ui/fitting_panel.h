#ifndef FITTING_PANEL_H
#define FITTING_PANEL_H

#include <string>
#include <vector>
#include <array>
#include <functional>
#include <optional>

namespace UI {

// Module information
struct ModuleInfo {
    std::string module_id;
    std::string name;
    std::string type;  // weapon, shield, armor, etc.
    float cpu_cost;
    float powergrid_cost;
    bool is_online;
    bool is_active;
    
    ModuleInfo() = default;
    ModuleInfo(const std::string& id, const std::string& n, const std::string& t,
               float cpu, float pg, bool online = false, bool active = false)
        : module_id(id), name(n), type(t), cpu_cost(cpu), powergrid_cost(pg),
          is_online(online), is_active(active) {}
};

// Ship fitting data
struct FittingData {
    std::string ship_name = "Unknown Ship";
    std::string ship_type = "Frigate";
    
    // Resources
    float cpu_used = 0.0f;
    float cpu_max = 100.0f;
    float powergrid_used = 0.0f;
    float powergrid_max = 50.0f;
    
    // Slots (nullptr means empty slot)
    std::array<std::optional<ModuleInfo>, 8> high_slots;
    std::array<std::optional<ModuleInfo>, 8> mid_slots;
    std::array<std::optional<ModuleInfo>, 8> low_slots;
    std::array<std::optional<ModuleInfo>, 3> rig_slots;
};

// Callback types
using FitModuleCallback = std::function<void(const std::string& module_id, const std::string& slot_type, int slot_index)>;
using UnfitModuleCallback = std::function<void(const std::string& slot_type, int slot_index)>;
using OnlineModuleCallback = std::function<void(const std::string& slot_type, int slot_index, bool online)>;

class FittingPanel {
public:
    FittingPanel();
    ~FittingPanel() = default;
    
    // Render the fitting panel
    void Render();
    
    // Render just the panel contents (no Begin/End) — used by docking manager
    void RenderContents();
    
    // Update fitting data
    void SetFittingData(const FittingData& data);
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetFitModuleCallback(FitModuleCallback callback) { m_onFitModule = callback; }
    void SetUnfitModuleCallback(UnfitModuleCallback callback) { m_onUnfitModule = callback; }
    void SetOnlineModuleCallback(OnlineModuleCallback callback) { m_onOnlineModule = callback; }
    
    // Response feedback methods
    void ShowSuccess(const std::string& message);
    void ShowError(const std::string& message);
    void SetPendingOperation(bool pending) { m_pendingOperation = pending; }
    
private:
    bool m_visible;
    FittingData m_data;
    
    // Callbacks
    FitModuleCallback m_onFitModule;
    UnfitModuleCallback m_onUnfitModule;
    OnlineModuleCallback m_onOnlineModule;
    
    // Response feedback state
    bool m_pendingOperation;
    std::string m_feedbackMessage;
    bool m_feedbackIsError;
    float m_feedbackTimer;
    
    // Helper functions
    void RenderShipInfo();
    void RenderResourceBars();
    void RenderSlotSection(const char* title, const std::array<std::optional<ModuleInfo>, 8>& slots, 
                           const std::string& slot_type, int max_slots);
    void RenderRigSection();
    void RenderModuleSlot(const std::optional<ModuleInfo>& module, const std::string& slot_type, 
                          int slot_index, bool enabled);
    void RenderResourceBar(const char* label, float used, float max, float y_offset);
};

} // namespace UI

#endif // FITTING_PANEL_H
