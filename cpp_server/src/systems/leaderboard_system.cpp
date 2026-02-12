#include "systems/leaderboard_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

LeaderboardSystem::LeaderboardSystem(ecs::World* world)
    : System(world) {
}

void LeaderboardSystem::update(float /*delta_time*/) {
    // Leaderboard updates are event-driven via record* methods
}

int LeaderboardSystem::findOrCreateEntry(const std::string& entity_id,
                                          const std::string& player_id,
                                          const std::string& player_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return -1;

    for (int i = 0; i < static_cast<int>(board->entries.size()); ++i) {
        if (board->entries[i].player_id == player_id) return i;
    }

    components::Leaderboard::PlayerEntry entry;
    entry.player_id = player_id;
    entry.player_name = player_name;
    board->entries.push_back(entry);
    return static_cast<int>(board->entries.size()) - 1;
}

void LeaderboardSystem::recordKill(const std::string& entity_id,
                                    const std::string& player_id,
                                    const std::string& player_name) {
    int idx = findOrCreateEntry(entity_id, player_id, player_name);
    if (idx < 0) return;

    auto* entity = world_->getEntity(entity_id);
    auto* board = entity->getComponent<components::Leaderboard>();
    board->entries[idx].total_kills++;
}

void LeaderboardSystem::recordIskEarned(const std::string& entity_id,
                                         const std::string& player_id,
                                         const std::string& player_name,
                                         double amount) {
    int idx = findOrCreateEntry(entity_id, player_id, player_name);
    if (idx < 0) return;

    auto* entity = world_->getEntity(entity_id);
    auto* board = entity->getComponent<components::Leaderboard>();
    board->entries[idx].total_isk_earned += amount;
}

void LeaderboardSystem::recordMissionComplete(const std::string& entity_id,
                                               const std::string& player_id,
                                               const std::string& player_name) {
    int idx = findOrCreateEntry(entity_id, player_id, player_name);
    if (idx < 0) return;

    auto* entity = world_->getEntity(entity_id);
    auto* board = entity->getComponent<components::Leaderboard>();
    board->entries[idx].missions_completed++;
}

void LeaderboardSystem::recordTournamentWin(const std::string& entity_id,
                                             const std::string& player_id,
                                             const std::string& player_name) {
    int idx = findOrCreateEntry(entity_id, player_id, player_name);
    if (idx < 0) return;

    auto* entity = world_->getEntity(entity_id);
    auto* board = entity->getComponent<components::Leaderboard>();
    board->entries[idx].tournaments_won++;
}

void LeaderboardSystem::recordDamageDealt(const std::string& entity_id,
                                           const std::string& player_id,
                                           const std::string& player_name,
                                           double amount) {
    int idx = findOrCreateEntry(entity_id, player_id, player_name);
    if (idx < 0) return;

    auto* entity = world_->getEntity(entity_id);
    auto* board = entity->getComponent<components::Leaderboard>();
    board->entries[idx].total_damage_dealt += amount;
}

int LeaderboardSystem::getPlayerKills(const std::string& entity_id,
                                       const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0;

    for (const auto& e : board->entries) {
        if (e.player_id == player_id) return e.total_kills;
    }
    return 0;
}

double LeaderboardSystem::getPlayerIskEarned(const std::string& entity_id,
                                              const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0.0;

    for (const auto& e : board->entries) {
        if (e.player_id == player_id) return e.total_isk_earned;
    }
    return 0.0;
}

int LeaderboardSystem::getPlayerMissions(const std::string& entity_id,
                                          const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0;

    for (const auto& e : board->entries) {
        if (e.player_id == player_id) return e.missions_completed;
    }
    return 0;
}

void LeaderboardSystem::defineAchievement(const std::string& entity_id,
                                           const std::string& achievement_id,
                                           const std::string& name,
                                           const std::string& description,
                                           const std::string& category,
                                           const std::string& stat_key,
                                           int requirement) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return;

    components::Leaderboard::Achievement ach;
    ach.achievement_id = achievement_id;
    ach.name = name;
    ach.description = description;
    ach.category = category;
    ach.stat_key = stat_key;
    ach.requirement = requirement;
    board->achievements.push_back(ach);
}

int LeaderboardSystem::checkAchievements(const std::string& entity_id,
                                          const std::string& player_id,
                                          float current_time) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0;

    // Find player entry
    const components::Leaderboard::PlayerEntry* pe = nullptr;
    for (const auto& e : board->entries) {
        if (e.player_id == player_id) { pe = &e; break; }
    }
    if (!pe) return 0;

    int newly_unlocked = 0;

    for (const auto& ach : board->achievements) {
        // Already unlocked?
        bool already = false;
        for (const auto& u : board->unlocked) {
            if (u.achievement_id == ach.achievement_id && u.player_id == player_id) {
                already = true;
                break;
            }
        }
        if (already) continue;

        // Check requirement against stat
        int stat_value = 0;
        if (ach.stat_key == "total_kills") stat_value = pe->total_kills;
        else if (ach.stat_key == "missions_completed") stat_value = pe->missions_completed;
        else if (ach.stat_key == "tournaments_won") stat_value = pe->tournaments_won;
        else if (ach.stat_key == "ships_destroyed") stat_value = pe->ships_destroyed;
        else if (ach.stat_key == "total_isk_earned") stat_value = static_cast<int>(pe->total_isk_earned);
        else if (ach.stat_key == "total_damage_dealt") stat_value = static_cast<int>(pe->total_damage_dealt);

        if (stat_value >= ach.requirement) {
            components::Leaderboard::UnlockedAchievement ua;
            ua.achievement_id = ach.achievement_id;
            ua.player_id = player_id;
            ua.unlock_time = current_time;
            board->unlocked.push_back(ua);
            newly_unlocked++;
        }
    }

    return newly_unlocked;
}

bool LeaderboardSystem::hasAchievement(const std::string& entity_id,
                                        const std::string& player_id,
                                        const std::string& achievement_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return false;

    for (const auto& u : board->unlocked) {
        if (u.achievement_id == achievement_id && u.player_id == player_id) return true;
    }
    return false;
}

int LeaderboardSystem::getPlayerAchievementCount(const std::string& entity_id,
                                                  const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0;

    int count = 0;
    for (const auto& u : board->unlocked) {
        if (u.player_id == player_id) count++;
    }
    return count;
}

int LeaderboardSystem::getEntryCount(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return 0;

    return static_cast<int>(board->entries.size());
}

std::vector<std::string> LeaderboardSystem::getRankingByKills(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return {};

    auto* board = entity->getComponent<components::Leaderboard>();
    if (!board) return {};

    // Copy entries, sort by kills descending
    auto sorted = board->entries;
    std::sort(sorted.begin(), sorted.end(),
              [](const components::Leaderboard::PlayerEntry& a,
                 const components::Leaderboard::PlayerEntry& b) {
                  return a.total_kills > b.total_kills;
              });

    std::vector<std::string> result;
    result.reserve(sorted.size());
    for (const auto& e : sorted) {
        result.push_back(e.player_id);
    }
    return result;
}

} // namespace systems
} // namespace atlas
