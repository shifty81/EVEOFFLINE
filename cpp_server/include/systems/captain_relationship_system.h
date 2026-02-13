#ifndef EVE_SYSTEMS_CAPTAIN_RELATIONSHIP_SYSTEM_H
#define EVE_SYSTEMS_CAPTAIN_RELATIONSHIP_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

class CaptainRelationshipSystem : public ecs::System {
public:
    explicit CaptainRelationshipSystem(ecs::World* world);
    ~CaptainRelationshipSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "CaptainRelationshipSystem"; }

    void recordEvent(const std::string& entity_id, const std::string& other_id, const std::string& event_type);
    float getAffinity(const std::string& entity_id, const std::string& other_id) const;
    std::string getRelationshipStatus(const std::string& entity_id, const std::string& other_id) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_CAPTAIN_RELATIONSHIP_SYSTEM_H
