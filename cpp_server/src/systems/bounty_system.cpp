#include "systems/bounty_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <memory>

namespace eve {
namespace systems {

BountySystem::BountySystem(ecs::World* world)
    : System(world) {
}

void BountySystem::update(float /*delta_time*/) {
    // Bounty processing is event-driven via processKill()
}

double BountySystem::processKill(const std::string& killer_id,
                                  const std::string& target_id,
                                  const std::string& target_name,
                                  double bounty_amount,
                                  const std::string& faction) {
    auto* entity = world_->getEntity(killer_id);
    if (!entity) return 0.0;

    auto* player = entity->getComponent<components::Player>();
    if (!player) return 0.0;

    // Auto-create BountyLedger if missing
    auto* ledger = entity->getComponent<components::BountyLedger>();
    if (!ledger) {
        auto newLedger = std::make_unique<components::BountyLedger>();
        ledger = newLedger.get();
        entity->addComponent(std::move(newLedger));
    }

    // Award bounty ISK
    player->isk += bounty_amount;

    // Update ledger
    ledger->total_bounty_earned += bounty_amount;
    ledger->total_kills++;

    // Add record
    components::BountyLedger::BountyRecord record;
    record.target_id = target_id;
    record.target_name = target_name;
    record.bounty_amount = bounty_amount;
    record.faction = faction;
    ledger->recent_kills.push_back(record);

    // Trim to MAX_RECENT
    while (static_cast<int>(ledger->recent_kills.size()) > components::BountyLedger::MAX_RECENT) {
        ledger->recent_kills.erase(ledger->recent_kills.begin());
    }

    return bounty_amount;
}

double BountySystem::getTotalBounty(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0;

    auto* ledger = entity->getComponent<components::BountyLedger>();
    if (!ledger) return 0.0;

    return ledger->total_bounty_earned;
}

int BountySystem::getTotalKills(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* ledger = entity->getComponent<components::BountyLedger>();
    if (!ledger) return 0;

    return ledger->total_kills;
}

} // namespace systems
} // namespace eve
