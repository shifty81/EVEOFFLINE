#include "systems/mission_template_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <functional>

namespace atlas {
namespace systems {

MissionTemplateSystem::MissionTemplateSystem(ecs::World* world)
    : System(world) {
}

void MissionTemplateSystem::update(float /*delta_time*/) {
    // Templates are static data; nothing to tick.
}

// ---------------------------------------------------------------------------
// Helper – create a template entity
// ---------------------------------------------------------------------------

void MissionTemplateSystem::addTemplate(
        const std::string& template_id,
        const std::string& name_pattern,
        const std::string& type,
        int level,
        const std::string& required_faction,
        float min_standing,
        const std::vector<components::MissionTemplate::ObjectiveTemplate>& objectives,
        double base_isk,
        double isk_per_level,
        float base_standing_reward,
        float standing_per_level,
        float base_time_limit) {

    std::string entity_id = "mission_tpl_" + std::to_string(template_counter_++);
    auto* entity = world_->createEntity(entity_id);
    if (!entity) return;

    auto comp = std::make_unique<components::MissionTemplate>();
    auto* tpl  = comp.get();
    entity->addComponent(std::move(comp));

    tpl->template_id          = template_id;
    tpl->name_pattern          = name_pattern;
    tpl->type                  = type;
    tpl->level                 = level;
    tpl->required_faction      = required_faction;
    tpl->min_standing          = min_standing;
    tpl->objective_templates   = objectives;
    tpl->base_isk              = base_isk;
    tpl->isk_per_level         = isk_per_level;
    tpl->base_standing_reward  = base_standing_reward;
    tpl->standing_per_level    = standing_per_level;
    tpl->base_time_limit       = base_time_limit;
}

// ---------------------------------------------------------------------------
// installDefaultTemplates – 16 templates across 5 mission types
// ---------------------------------------------------------------------------

void MissionTemplateSystem::installDefaultTemplates() {
    using OT = components::MissionTemplate::ObjectiveTemplate;

    // ---- Combat L1-L5 ----
    addTemplate("combat_l1", "Pirate Clearance: {system}", "combat", 1, "", 0.0f,
                {OT{"destroy", "pirate_frigate", 3, 5}},
                100000.0, 50000.0, 0.1f, 0.05f, 3600.0f);

    addTemplate("combat_l2", "Pirate Assault: {system}", "combat", 2, "", 0.0f,
                {OT{"destroy", "pirate_frigate", 5, 8}},
                150000.0, 50000.0, 0.1f, 0.05f, 3600.0f);

    addTemplate("combat_l3", "Pirate Incursion: {system}", "combat", 3, "", 0.5f,
                {OT{"destroy", "pirate_cruiser", 4, 8}},
                200000.0, 50000.0, 0.1f, 0.05f, 5400.0f);

    addTemplate("combat_l4", "Pirate Stronghold: {system}", "combat", 4, "", 1.0f,
                {OT{"destroy", "pirate_battleship", 5, 10}},
                500000.0, 100000.0, 0.15f, 0.05f, 7200.0f);

    addTemplate("combat_l5", "Pirate Armada: {system}", "combat", 5, "", 2.0f,
                {OT{"destroy", "pirate_battleship", 10, 15}},
                1000000.0, 200000.0, 0.2f, 0.1f, 10800.0f);

    // ---- Mining L1-L3 ----
    addTemplate("mining_l1", "Ore Requisition: {system}", "mining", 1, "", 0.0f,
                {OT{"mine", "Veldspar", 100, 200}},
                50000.0, 25000.0, 0.05f, 0.03f, 3600.0f);

    addTemplate("mining_l2", "Deep Core Request: {system}", "mining", 2, "", 0.0f,
                {OT{"mine", "Scordite", 200, 350}},
                75000.0, 25000.0, 0.05f, 0.03f, 5400.0f);

    addTemplate("mining_l3", "Rare Ore Extraction: {system}", "mining", 3, "", 0.3f,
                {OT{"mine", "Kernite", 350, 500}},
                120000.0, 30000.0, 0.08f, 0.04f, 7200.0f);

    // ---- Courier L1-L3 ----
    addTemplate("courier_l1", "Supply Run: {system}", "courier", 1, "", 0.0f,
                {OT{"deliver", "Trade Goods", 1, 3}},
                60000.0, 20000.0, 0.05f, 0.02f, 3600.0f);

    addTemplate("courier_l2", "Priority Delivery: {system}", "courier", 2, "", 0.0f,
                {OT{"deliver", "Medical Supplies", 2, 5}},
                90000.0, 25000.0, 0.07f, 0.03f, 5400.0f);

    addTemplate("courier_l3", "Emergency Freight: {system}", "courier", 3, "", 0.5f,
                {OT{"deliver", "Munitions", 3, 7}},
                140000.0, 30000.0, 0.1f, 0.04f, 7200.0f);

    // ---- Trade L1-L2 ----
    addTemplate("trade_l1", "Market Opportunity: {system}", "trade", 1, "", 0.0f,
                {OT{"deliver", "Consumer Electronics", 1, 3}},
                80000.0, 30000.0, 0.05f, 0.02f, -1.0f);

    addTemplate("trade_l2", "Bulk Trade Deal: {system}", "trade", 2, "", 0.2f,
                {OT{"deliver", "Luxury Goods", 2, 5}},
                150000.0, 40000.0, 0.08f, 0.03f, -1.0f);

    // ---- Exploration L1-L3 ----
    addTemplate("exploration_l1", "Survey Anomaly: {system}", "exploration", 1, "", 0.0f,
                {OT{"reach", "anomaly_site", 1, 2}},
                70000.0, 20000.0, 0.05f, 0.02f, 3600.0f);

    addTemplate("exploration_l2", "Deep Scan: {system}", "exploration", 2, "", 0.0f,
                {OT{"reach", "data_site", 2, 3}},
                100000.0, 25000.0, 0.07f, 0.03f, 5400.0f);

    addTemplate("exploration_l3", "Relic Recovery: {system}", "exploration", 3, "", 0.5f,
                {OT{"reach", "relic_site", 2, 4}},
                160000.0, 35000.0, 0.1f, 0.04f, 7200.0f);
}

// ---------------------------------------------------------------------------
// getTemplatesForFaction
// ---------------------------------------------------------------------------

std::vector<std::string> MissionTemplateSystem::getTemplatesForFaction(
        const std::string& faction,
        float standing,
        int level) const {

    std::vector<std::string> result;

    for (auto* entity : world_->getEntities<components::MissionTemplate>()) {
        auto* tpl = entity->getComponent<components::MissionTemplate>();
        if (!tpl) continue;

        // Level must match exactly
        if (tpl->level != level) continue;

        // Faction filter: empty required_faction matches any faction
        if (!tpl->required_faction.empty() && tpl->required_faction != faction)
            continue;

        // Standing check
        if (standing < tpl->min_standing) continue;

        result.push_back(tpl->template_id);
    }

    return result;
}

// ---------------------------------------------------------------------------
// generateMissionFromTemplate
// ---------------------------------------------------------------------------

components::MissionTracker::ActiveMission
MissionTemplateSystem::generateMissionFromTemplate(
        const std::string& template_id,
        const std::string& system_id,
        const std::string& /*player_entity_id*/) const {

    components::MissionTracker::ActiveMission mission;

    // Find the template
    const components::MissionTemplate* tpl = nullptr;
    for (auto* entity : world_->getEntities<components::MissionTemplate>()) {
        auto* candidate = entity->getComponent<components::MissionTemplate>();
        if (candidate && candidate->template_id == template_id) {
            tpl = candidate;
            break;
        }
    }

    if (!tpl) return mission; // empty mission if template not found

    // Deterministic seed from system_id + template_id
    std::hash<std::string> hasher;
    size_t seed = hasher(system_id + template_id);

    // Fill mission fields
    mission.mission_id  = template_id + "_" + system_id;

    // Substitute {system} in name_pattern
    std::string name = tpl->name_pattern;
    auto pos = name.find("{system}");
    if (pos != std::string::npos)
        name.replace(pos, 8, system_id);
    mission.name = name;

    mission.level         = tpl->level;
    mission.type          = tpl->type;
    mission.agent_faction = tpl->required_faction;

    // Rewards
    mission.isk_reward      = tpl->base_isk + tpl->level * tpl->isk_per_level;
    mission.standing_reward = tpl->base_standing_reward + tpl->level * tpl->standing_per_level;
    mission.time_remaining  = tpl->base_time_limit;

    // Build objectives with deterministic random counts
    uint32_t rng = static_cast<uint32_t>(seed);
    for (const auto& ot : tpl->objective_templates) {
        components::MissionTracker::Objective obj;
        obj.type   = ot.type;
        obj.target = ot.target;

        int range = ot.count_max - ot.count_min + 1;
        if (range <= 0) range = 1;
        obj.required  = ot.count_min + static_cast<int>(rng % static_cast<uint32_t>(range));
        obj.completed = 0;

        // Advance RNG (simple LCG step)
        rng = rng * 1664525u + 1013904223u;

        mission.objectives.push_back(std::move(obj));
    }

    return mission;
}

} // namespace systems
} // namespace atlas
