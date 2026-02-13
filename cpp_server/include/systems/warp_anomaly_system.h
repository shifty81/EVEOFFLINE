#ifndef EVE_SYSTEMS_WARP_ANOMALY_SYSTEM_H
#define EVE_SYSTEMS_WARP_ANOMALY_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <unordered_map>

namespace atlas {
namespace systems {

struct WarpAnomaly {
    std::string anomaly_id;
    std::string name;
    std::string description;
    std::string category = "visual";  // visual/sensory/shear/legendary
    float duration = 5.0f;
};

class WarpAnomalySystem : public ecs::System {
public:
    explicit WarpAnomalySystem(ecs::World* world);
    ~WarpAnomalySystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WarpAnomalySystem"; }

    bool tryTriggerAnomaly(const std::string& entity_id);
    WarpAnomaly getLastAnomaly(const std::string& entity_id) const;
    void clearAnomaly(const std::string& entity_id);
    int getAnomalyCount(const std::string& entity_id) const;

private:
    std::unordered_map<std::string, WarpAnomaly> last_anomalies_;
    std::unordered_map<std::string, int> anomaly_counts_;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WARP_ANOMALY_SYSTEM_H
