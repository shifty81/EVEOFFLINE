#include "systems/corporation_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <cctype>

namespace eve {
namespace systems {

CorporationSystem::CorporationSystem(ecs::World* world)
    : System(world) {
}

void CorporationSystem::update(float /*delta_time*/) {
}

bool CorporationSystem::createCorporation(const std::string& entity_id,
                                          const std::string& corp_name,
                                          const std::string& ticker) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* player = entity->getComponent<components::Player>();
    if (!player) return false;

    // Build corp entity ID: "corp_" + lowercased name with spaces replaced by underscores
    std::string corp_id = "corp_";
    for (char c : corp_name) {
        if (c == ' ')
            corp_id += '_';
        else
            corp_id += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    auto* corp_entity = world_->createEntity(corp_id);
    if (!corp_entity) return false;

    auto corp = std::make_unique<components::Corporation>();
    corp->corp_id = corp_id;
    corp->corp_name = corp_name;
    corp->ticker = ticker;
    corp->ceo_id = entity_id;
    corp->tax_rate = 0.05f;
    corp->member_ids.push_back(entity_id);
    corp_entity->addComponent(std::move(corp));

    player->corporation = corp_name;

    return true;
}

bool CorporationSystem::joinCorporation(const std::string& player_entity_id,
                                        const std::string& corp_entity_id) {
    auto* player_entity = world_->getEntity(player_entity_id);
    if (!player_entity) return false;

    auto* player = player_entity->getComponent<components::Player>();
    if (!player) return false;

    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return false;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return false;

    // Check for duplicates
    for (const auto& id : corp->member_ids) {
        if (id == player_entity_id) return false;
    }

    corp->member_ids.push_back(player_entity_id);
    player->corporation = corp->corp_name;

    return true;
}

bool CorporationSystem::leaveCorporation(const std::string& player_entity_id,
                                         const std::string& corp_entity_id) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return false;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return false;

    if (corp->ceo_id == player_entity_id) return false;

    auto it = std::find(corp->member_ids.begin(), corp->member_ids.end(), player_entity_id);
    if (it == corp->member_ids.end()) return false;

    corp->member_ids.erase(it);

    auto* player_entity = world_->getEntity(player_entity_id);
    if (player_entity) {
        auto* player = player_entity->getComponent<components::Player>();
        if (player) {
            player->corporation = "NPC Corp";
        }
    }

    return true;
}

bool CorporationSystem::setTaxRate(const std::string& corp_entity_id,
                                   const std::string& requester_id,
                                   float rate) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return false;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return false;

    if (corp->ceo_id != requester_id) return false;

    if (rate < 0.0f) rate = 0.0f;
    if (rate > 1.0f) rate = 1.0f;

    corp->tax_rate = rate;
    return true;
}

double CorporationSystem::applyTax(const std::string& corp_entity_id, double income) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return income;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return income;

    double tax = income * corp->tax_rate;
    corp->corp_wallet += tax;
    return income - tax;
}

int CorporationSystem::getMemberCount(const std::string& corp_entity_id) {
    auto* corp_entity = world_->getEntity(corp_entity_id);
    if (!corp_entity) return 0;

    auto* corp = corp_entity->getComponent<components::Corporation>();
    if (!corp) return 0;

    return static_cast<int>(corp->member_ids.size());
}

} // namespace systems
} // namespace eve
