#ifndef EVE_SYSTEMS_CORPORATION_SYSTEM_H
#define EVE_SYSTEMS_CORPORATION_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace eve {
namespace systems {

class CorporationSystem : public ecs::System {
public:
    explicit CorporationSystem(ecs::World* world);
    ~CorporationSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CorporationSystem"; }

    bool createCorporation(const std::string& entity_id,
                           const std::string& corp_name,
                           const std::string& ticker);

    bool joinCorporation(const std::string& player_entity_id,
                         const std::string& corp_entity_id);

    bool leaveCorporation(const std::string& player_entity_id,
                          const std::string& corp_entity_id);

    bool setTaxRate(const std::string& corp_entity_id,
                    const std::string& requester_id,
                    float rate);

    double applyTax(const std::string& corp_entity_id, double income);

    int getMemberCount(const std::string& corp_entity_id);
};

} // namespace systems
} // namespace eve

#endif // EVE_SYSTEMS_CORPORATION_SYSTEM_H
