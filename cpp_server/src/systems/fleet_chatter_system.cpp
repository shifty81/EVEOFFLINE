#include "systems/fleet_chatter_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <functional>
#include <vector>
#include <array>

namespace atlas {
namespace systems {

// Chatter line pools per activity
static const std::vector<std::string>& getPool(const std::string& activity) {
    static const std::vector<std::string> warp_lines = {
        "Quiet today, boss.",
        "Tunnel's smooth this run.",
        "Ever wonder what's between lanes?",
        "Long haul... I like it.",
        "Still can't believe we made it out of that last one."
    };
    static const std::vector<std::string> mining_lines = {
        "Cargo's getting full.",
        "Feels strange pulling metal out of a dead world.",
        "Never thought I'd miss gunfire.",
        "Yield's decent here.",
        "Another load. Same as the last."
    };
    static const std::vector<std::string> combat_lines = {
        "Shields holding.",
        "That was too close.",
        "Focus fire!",
        "We've got this.",
        "Watch your six!"
    };
    static const std::vector<std::string> idle_lines = {
        "Quiet today.",
        "Guess we're just flying.",
        "You alright up there?",
        "Map says empty. Space never is.",
        "Nothing on scan."
    };
    static const std::vector<std::string> travel_lines = {
        "How far out are we?",
        "Nice sector.",
        "This place feels different.",
        "Autopilot's steady.",
        "Should be there soon."
    };

    if (activity == "Warp") return warp_lines;
    if (activity == "Mining") return mining_lines;
    if (activity == "Combat") return combat_lines;
    if (activity == "Travel") return travel_lines;
    return idle_lines;
}

FleetChatterSystem::FleetChatterSystem(ecs::World* world)
    : System(world) {
}

void FleetChatterSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FleetChatterState>();
    for (auto* entity : entities) {
        auto* chatter = entity->getComponent<components::FleetChatterState>();
        if (chatter && chatter->chatter_cooldown > 0.0f) {
            chatter->chatter_cooldown -= delta_time;
            if (chatter->chatter_cooldown < 0.0f) {
                chatter->chatter_cooldown = 0.0f;
            }
        }
    }
}

void FleetChatterSystem::setActivity(const std::string& entity_id, const std::string& activity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* chatter = entity->getComponent<components::FleetChatterState>();
    if (!chatter) {
        entity->addComponent(std::make_unique<components::FleetChatterState>());
        chatter = entity->getComponent<components::FleetChatterState>();
    }

    chatter->current_activity = activity;
}

std::string FleetChatterSystem::getNextChatterLine(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* chatter = entity->getComponent<components::FleetChatterState>();
    if (!chatter) {
        entity->addComponent(std::make_unique<components::FleetChatterState>());
        chatter = entity->getComponent<components::FleetChatterState>();
    }

    if (chatter->chatter_cooldown > 0.0f) {
        return "";
    }

    const auto& pool = getPool(chatter->current_activity);
    if (pool.empty()) return "";

    // Deterministic selection based on entity_id hash + lines spoken
    std::hash<std::string> hasher;
    size_t hash_val = hasher(entity_id) + static_cast<size_t>(chatter->lines_spoken_total);
    size_t index = hash_val % pool.size();

    // Personality modifies cooldown
    float base_cooldown = 25.0f;
    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (personality) {
        // sociability < 0.3 doubles cooldown
        if (personality->sociability < 0.3f) {
            base_cooldown *= 2.0f;
        }
        // Use optimism to shift within 25-45 range
        // High optimism -> shorter cooldown, low optimism -> longer cooldown
        float range_offset = (1.0f - personality->optimism) * 20.0f;
        base_cooldown += range_offset;
    } else {
        base_cooldown = 35.0f;  // default mid-range
    }

    chatter->chatter_cooldown = base_cooldown;
    chatter->lines_spoken_total++;
    chatter->last_line_spoken = pool[index];
    chatter->is_speaking = true;

    return pool[index];
}

bool FleetChatterSystem::isOnCooldown(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    const auto* chatter = entity->getComponent<components::FleetChatterState>();
    if (!chatter) return false;

    return chatter->chatter_cooldown > 0.0f;
}

int FleetChatterSystem::getTotalLinesSpoken(const std::string& entity_id) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    const auto* chatter = entity->getComponent<components::FleetChatterState>();
    if (!chatter) return 0;

    return chatter->lines_spoken_total;
}

} // namespace systems
} // namespace atlas
