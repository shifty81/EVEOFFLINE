#include "systems/fleet_chatter_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <functional>
#include <vector>
#include <array>
#include <algorithm>

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
            if (chatter->chatter_cooldown <= 0.0f) {
                chatter->chatter_cooldown = 0.0f;
                chatter->is_speaking = false;  // speech finished
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

    // Timing rule: no overlap — only one speaker at a time in the fleet
    if (isAnyoneSpeaking()) {
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

    // Clamp cooldown to 20-45s timing rule
    base_cooldown = std::clamp(base_cooldown, 20.0f, 45.0f);

    chatter->chatter_cooldown = base_cooldown;
    chatter->lines_spoken_total++;
    chatter->last_line_spoken = pool[index];
    chatter->is_speaking = true;
    chatter->speaking_priority = 1.0f;  // normal priority

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

// ---------------------------------------------------------------------------
// Personality-contextual line pools
// ---------------------------------------------------------------------------

// Dominant-trait categories: aggressive, cautious, optimistic, professional
static const std::vector<std::string>& getAggressivePool(const std::string& activity) {
    static const std::vector<std::string> combat = {
        "Let me at them!", "More targets — good.", "Weapons hot, always.",
        "They won't know what hit them.", "I live for this."
    };
    static const std::vector<std::string> other = {
        "When's the next fight?", "This peace won't last.",
        "I'd rather be shooting.", "Staying sharp, boss.", "Ready for anything."
    };
    if (activity == "Combat") return combat;
    return other;
}

static const std::vector<std::string>& getCautiousPool(const std::string& activity) {
    static const std::vector<std::string> combat = {
        "Careful now.", "Watch the flanks.", "Don't overcommit.",
        "We should pull back if this gets worse.", "Shields first."
    };
    static const std::vector<std::string> other = {
        "I don't like the look of this sector.", "Stay alert.",
        "Something feels off.", "Let's not linger.", "Running diagnostics."
    };
    if (activity == "Combat") return combat;
    return other;
}

static const std::vector<std::string>& getOptimisticPool(const std::string& activity) {
    static const std::vector<std::string> combat = {
        "We've got this!", "Almost there!", "Together we're unstoppable!",
        "Just a scratch!", "Victory's close, I can feel it."
    };
    static const std::vector<std::string> other = {
        "Beautiful day for flying.", "Things are looking up.",
        "Glad to be out here with you.", "Could be worse, right?", "Onward and upward."
    };
    if (activity == "Combat") return combat;
    return other;
}

static const std::vector<std::string>& getProfessionalPool(const std::string& activity) {
    static const std::vector<std::string> combat = {
        "Engaging hostiles, standing by.", "Target acquired.",
        "Damage report coming.", "Maintaining formation.", "Copy that, commander."
    };
    static const std::vector<std::string> other = {
        "All systems nominal.", "Awaiting orders.",
        "Course steady.", "Proceeding as planned.", "Status green."
    };
    if (activity == "Combat") return combat;
    return other;
}

static const std::vector<std::string>& getPersonalityPool(
    const components::CaptainPersonality& p, const std::string& activity) {

    // Pick the dominant trait
    float maxTrait = p.aggression;
    int dominant = 0; // 0=aggressive, 1=cautious(paranoia), 2=optimistic, 3=professional
    if (p.paranoia > maxTrait) { maxTrait = p.paranoia; dominant = 1; }
    if (p.optimism > maxTrait) { maxTrait = p.optimism; dominant = 2; }
    if (p.professionalism > maxTrait) { maxTrait = p.professionalism; dominant = 3; }

    switch (dominant) {
        case 0: return getAggressivePool(activity);
        case 1: return getCautiousPool(activity);
        case 2: return getOptimisticPool(activity);
        case 3: return getProfessionalPool(activity);
        default: return getAggressivePool(activity);
    }
}

std::string FleetChatterSystem::getContextualLine(const std::string& entity_id) {
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

    // Timing rule: no overlap — only one speaker at a time
    if (isAnyoneSpeaking()) {
        return "";
    }

    auto* personality = entity->getComponent<components::CaptainPersonality>();
    if (!personality) {
        // Fall back to generic pool
        return getNextChatterLine(entity_id);
    }

    const auto& pool = getPersonalityPool(*personality, chatter->current_activity);
    if (pool.empty()) return "";

    std::hash<std::string> hasher;
    size_t hash_val = hasher(entity_id) + static_cast<size_t>(chatter->lines_spoken_total);
    size_t index = hash_val % pool.size();

    // Personality-based cooldown (same formula as generic path)
    float base_cooldown = 25.0f;
    if (personality->sociability < 0.3f) {
        base_cooldown *= 2.0f;
    }
    float range_offset = (1.0f - personality->optimism) * 20.0f;
    base_cooldown += range_offset;

    // Clamp cooldown to 20-45s timing rule
    base_cooldown = std::clamp(base_cooldown, 20.0f, 45.0f);

    chatter->chatter_cooldown = base_cooldown;
    chatter->lines_spoken_total++;
    chatter->last_line_spoken = pool[index];
    chatter->is_speaking = true;
    chatter->speaking_priority = 1.0f;  // normal priority

    return pool[index];
}

// ---------------------------------------------------------------------------
// Phase 9: Interruptible chatter
// ---------------------------------------------------------------------------

bool FleetChatterSystem::interruptChatter(const std::string& entity_id, float new_priority) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* chatter = entity->getComponent<components::FleetChatterState>();
    if (!chatter) return false;

    if (!chatter->is_speaking) return false;

    // Interrupt only if new event has strictly higher priority
    if (new_priority > chatter->speaking_priority) {
        chatter->is_speaking = false;
        chatter->was_interrupted = true;
        chatter->chatter_cooldown = 0.0f;  // allow immediate re-speak
        return true;
    }
    return false;
}

bool FleetChatterSystem::isAnyoneSpeaking() const {
    auto entities = world_->getEntities<components::FleetChatterState>();
    for (const auto* entity : entities) {
        const auto* chatter = entity->getComponent<components::FleetChatterState>();
        if (chatter && chatter->is_speaking) {
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Phase 9: Silence interpretation
// ---------------------------------------------------------------------------

static const std::vector<std::string>& getSilenceLines() {
    static const std::vector<std::string> lines = {
        "Quiet today, boss.",
        "You alright up there?",
        "Haven't heard from you in a while.",
        "Everything okay, commander?",
        "Just checking in.",
    };
    return lines;
}

std::string FleetChatterSystem::getSilenceAwareLine(
    const std::string& entity_id,
    const std::string& player_entity_id) {

    // Check player silence threshold (120 seconds)
    auto* player_entity = world_->getEntity(player_entity_id);
    if (player_entity) {
        auto* presence = player_entity->getComponent<components::PlayerPresence>();
        if (presence && presence->time_since_last_command >= 120.0f) {
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

            const auto& pool = getSilenceLines();
            std::hash<std::string> hasher;
            size_t hash_val = hasher(entity_id) + static_cast<size_t>(chatter->lines_spoken_total);
            size_t index = hash_val % pool.size();

            chatter->chatter_cooldown = 45.0f;  // longer cooldown for silence comments
            chatter->lines_spoken_total++;
            chatter->last_line_spoken = pool[index];
            chatter->is_speaking = true;
            chatter->speaking_priority = 0.5f;   // low priority, can be interrupted

            return pool[index];
        }
    }

    // Fall back to personality-contextual line
    return getContextualLine(entity_id);
}

// ---------------------------------------------------------------------------
// Phase 9: Rumor propagation
// ---------------------------------------------------------------------------

void FleetChatterSystem::propagateRumor(const std::string& speaker_id,
                                         const std::string& listener_id) {
    auto* speaker = world_->getEntity(speaker_id);
    auto* listener = world_->getEntity(listener_id);
    if (!speaker || !listener) return;

    auto* speaker_log = speaker->getComponent<components::RumorLog>();
    if (!speaker_log || speaker_log->rumors.empty()) return;

    auto* listener_log = listener->getComponent<components::RumorLog>();
    if (!listener_log) {
        listener->addComponent(std::make_unique<components::RumorLog>());
        listener_log = listener->getComponent<components::RumorLog>();
    }

    // Pick the rumor with highest belief from the speaker
    const components::RumorLog::Rumor* best = nullptr;
    for (const auto& r : speaker_log->rumors) {
        if (!best || r.belief_strength > best->belief_strength) {
            best = &r;
        }
    }
    if (!best) return;

    // If listener already has this rumor, reinforce it
    for (auto& r : listener_log->rumors) {
        if (r.rumor_id == best->rumor_id) {
            r.times_heard++;
            r.belief_strength = std::min(r.belief_strength + 0.1f, 1.0f);
            return;
        }
    }

    // Otherwise copy it with halved belief (second-hand)
    listener_log->addRumor(best->rumor_id, best->text, false);
    for (auto& r : listener_log->rumors) {
        if (r.rumor_id == best->rumor_id) {
            r.belief_strength = best->belief_strength * 0.5f;
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Phase 9: Disagreement model
// ---------------------------------------------------------------------------

float FleetChatterSystem::computeDisagreement(const std::string& entity_id,
                                               float current_risk,
                                               bool task_mismatch) const {
    const auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    const auto* personality = entity->getComponent<components::CaptainPersonality>();
    const auto* morale = entity->getComponent<components::FleetMorale>();
    if (!personality) return 0.0f;

    float losses = 0.0f;
    if (morale) {
        losses = static_cast<float>(morale->losses);
    }

    // disagreement = risk × (1 - aggression) + losses × (1 - optimism) + mismatch
    float score = current_risk * (1.0f - personality->aggression)
                + losses * (1.0f - personality->optimism);

    if (task_mismatch) {
        score += 10.0f;
    }

    return std::max(score, 0.0f);
}

} // namespace systems
} // namespace atlas
