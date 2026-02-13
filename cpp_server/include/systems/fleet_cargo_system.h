#ifndef EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H
#define EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <cstdint>

namespace atlas {
namespace systems {

class FleetCargoSystem : public ecs::System {
public:
    explicit FleetCargoSystem(ecs::World* world);
    ~FleetCargoSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetCargoSystem"; }

    void addContributor(const std::string& pool_entity_id, const std::string& ship_entity_id);
    void removeContributor(const std::string& pool_entity_id, const std::string& ship_entity_id);
    uint64_t getTotalCapacity(const std::string& pool_entity_id) const;
    uint64_t getUsedCapacity(const std::string& pool_entity_id) const;
    void recalculate(const std::string& pool_entity_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_CARGO_SYSTEM_H
