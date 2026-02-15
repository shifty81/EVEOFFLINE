#ifndef EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H
#define EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

class TacticalOverlaySystem : public ecs::System {
public:
    explicit TacticalOverlaySystem(ecs::World* world);
    ~TacticalOverlaySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "TacticalOverlaySystem"; }

    void toggleOverlay(const std::string& entity_id);
    bool isEnabled(const std::string& entity_id) const;
    void setToolRange(const std::string& entity_id, float range, const std::string& tool_type);
    std::vector<float> getRingDistances(const std::string& entity_id) const;
    void setRingDistances(const std::string& entity_id, const std::vector<float>& distances);

    // Phase 10 additions

    /**
     * @brief Set the filter categories shown on the overlay.
     * These are shared with Overview and world brackets.
     */
    void setFilterCategories(const std::string& entity_id,
                             const std::vector<std::string>& categories);
    std::vector<std::string> getFilterCategories(const std::string& entity_id) const;

    /**
     * @brief Query whether the overlay is passive-display-only
     *        (no clickable elements, no dragging, no entity selection).
     */
    bool isPassiveDisplayOnly(const std::string& entity_id) const;

    /**
     * @brief Set the entity display priority for large-entity-count
     *        scaling (hostiles high-contrast, asteroids muted).
     */
    void setEntityDisplayPriority(const std::string& entity_id, float priority);
    float getEntityDisplayPriority(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_TACTICAL_OVERLAY_SYSTEM_H
