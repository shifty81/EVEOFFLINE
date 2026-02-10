#include "systems/insurance_system.h"
#include "ecs/world.h"
#include "components/game_components.h"

namespace eve {
namespace systems {

InsuranceSystem::InsuranceSystem(ecs::World* world)
    : System(world) {
}

void InsuranceSystem::update(float delta_time) {
    // Tick down duration on all InsurancePolicy components
    for (auto* entity : world_->getAllEntities()) {
        auto* policy = entity->getComponent<components::InsurancePolicy>();
        if (!policy || !policy->active) continue;

        if (policy->duration_remaining > 0.0f) {
            policy->duration_remaining -= delta_time;
            if (policy->duration_remaining <= 0.0f) {
                policy->duration_remaining = 0.0f;
                policy->active = false;
            }
        }
    }
}

bool InsuranceSystem::purchaseInsurance(const std::string& entity_id,
                                       const std::string& tier,
                                       double ship_value) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* player = entity->getComponent<components::Player>();
    if (!player) return false;

    // Determine tier parameters
    float coverage = 0.0f;
    float premium_rate = 0.0f;

    if (tier == "basic") {
        coverage = 0.5f;
        premium_rate = 0.1f;
    } else if (tier == "standard") {
        coverage = 0.7f;
        premium_rate = 0.2f;
    } else if (tier == "platinum") {
        coverage = 1.0f;
        premium_rate = 0.3f;
    } else {
        return false;
    }

    double premium = ship_value * premium_rate;
    if (player->isk < premium) return false;

    // Deduct premium
    player->isk -= premium;

    // Create policy component
    auto policy = std::make_unique<components::InsurancePolicy>();
    policy->policy_id = entity_id + "_" + tier;
    policy->ship_type = "ship";
    policy->tier = tier;
    policy->coverage_fraction = coverage;
    policy->premium_paid = premium;
    policy->payout_value = ship_value * coverage;
    policy->active = true;
    policy->claimed = false;

    entity->addComponent(std::move(policy));
    return true;
}

double InsuranceSystem::claimInsurance(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0;

    auto* policy = entity->getComponent<components::InsurancePolicy>();
    if (!policy || !policy->active || policy->claimed) return 0.0;

    auto* player = entity->getComponent<components::Player>();

    double payout = policy->payout_value;
    policy->claimed = true;

    if (player) {
        player->isk += payout;
    }

    return payout;
}

bool InsuranceSystem::hasActivePolicy(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* policy = entity->getComponent<components::InsurancePolicy>();
    if (!policy) return false;

    return policy->active && !policy->claimed;
}

} // namespace systems
} // namespace eve
