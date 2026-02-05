#ifndef MODULE_BROWSER_PANEL_H
#define MODULE_BROWSER_PANEL_H

#include <string>
#include <vector>
#include <functional>

namespace UI {

// Module database entry
struct ModuleBrowserEntry {
    std::string module_id;
    std::string name;
    std::string category;  // weapon, shield, armor, propulsion, etc.
    std::string type;      // specific type within category
    std::string description;
    float cpu_cost;
    float powergrid_cost;
    float meta_level;
    std::string slot_type; // high, mid, low, rig
    
    // Module stats (optional, depends on type)
    float damage = 0.0f;           // For weapons
    float shield_hp = 0.0f;        // For shield modules
    float armor_hp = 0.0f;         // For armor modules
    float speed_bonus = 0.0f;      // For propulsion modules
    float capacitor_use = 0.0f;    // For active modules
    float activation_time = 0.0f;  // For active modules
    
    ModuleBrowserEntry() = default;
    ModuleBrowserEntry(const std::string& id, const std::string& n, const std::string& cat,
                       const std::string& t, float cpu, float pg, const std::string& slot)
        : module_id(id), name(n), category(cat), type(t), cpu_cost(cpu), 
          powergrid_cost(pg), meta_level(0.0f), slot_type(slot) {}
};

// Callback types
using BrowseModuleCallback = std::function<void(const std::string& module_id)>;
using FitModuleFromBrowserCallback = std::function<void(const std::string& module_id)>;

class ModuleBrowserPanel {
public:
    ModuleBrowserPanel();
    ~ModuleBrowserPanel() = default;
    
    // Render the module browser panel
    void Render();
    
    // Set module database
    void SetModules(const std::vector<ModuleBrowserEntry>& modules);
    void AddModule(const ModuleBrowserEntry& module);
    void ClearModules();
    
    // Visibility
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
    // Callbacks
    void SetBrowseCallback(BrowseModuleCallback callback) { m_onBrowseModule = callback; }
    void SetFitCallback(FitModuleFromBrowserCallback callback) { m_onFitModule = callback; }
    
private:
    bool m_visible;
    std::vector<ModuleBrowserEntry> m_modules;
    std::vector<ModuleBrowserEntry> m_filteredModules;
    
    // Filter state
    char m_searchBuffer[128];
    std::string m_selectedCategory;
    std::string m_selectedSlotType;
    int m_sortMode;  // 0=name, 1=cpu, 2=powergrid, 3=meta
    
    // Selection
    int m_selectedIndex;
    
    // Callbacks
    BrowseModuleCallback m_onBrowseModule;
    FitModuleFromBrowserCallback m_onFitModule;
    
    // Helper functions
    void RenderSearchBar();
    void RenderFilters();
    void RenderModuleList();
    void RenderModuleDetails();
    void RenderModuleRow(const ModuleBrowserEntry& module, int index, bool selected);
    void ApplyFilters();
    void SortModules();
    std::vector<std::string> GetCategories() const;
    std::vector<std::string> GetSlotTypes() const;
};

} // namespace UI

#endif // MODULE_BROWSER_PANEL_H
