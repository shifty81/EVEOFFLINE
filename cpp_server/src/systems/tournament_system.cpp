#include "systems/tournament_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

TournamentSystem::TournamentSystem(ecs::World* world)
    : System(world) {
}

void TournamentSystem::update(float delta_time) {
    // Advance round timers for active tournaments
    auto entities = world_->getEntities<components::Tournament>();
    for (auto* entity : entities) {
        auto* tourney = entity->getComponent<components::Tournament>();
        if (!tourney || tourney->status != "active") continue;

        tourney->round_timer -= delta_time;
        if (tourney->round_timer <= 0.0f) {
            // Round ended — record result
            components::Tournament::RoundResult result;
            result.round_number = tourney->current_round;
            result.participant_count = 0;

            int best_score = -1;
            std::string best_id;
            for (const auto& p : tourney->participants) {
                if (!p.eliminated) {
                    result.participant_count++;
                    if (p.score > best_score) {
                        best_score = p.score;
                        best_id = p.player_id;
                    }
                }
            }
            result.winner_id = best_id;
            result.winner_score = best_score;
            tourney->round_results.push_back(result);

            // Advance to next round
            tourney->current_round++;
            if (tourney->current_round > tourney->total_rounds) {
                tourney->status = "completed";
            } else {
                tourney->round_timer = tourney->round_duration;
            }
        }
    }
}

bool TournamentSystem::createTournament(const std::string& entity_id,
                                         const std::string& tournament_id,
                                         const std::string& name,
                                         int max_participants,
                                         double entry_fee,
                                         float round_duration) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto tourney = std::make_unique<components::Tournament>();
    tourney->tournament_id = tournament_id;
    tourney->name = name;
    tourney->max_participants = max_participants;
    tourney->entry_fee = entry_fee;
    tourney->round_duration = round_duration;
    tourney->status = "registration";
    tourney->current_round = 0;
    tourney->total_rounds = 3;  // default 3 rounds
    tourney->prize_pool = 0.0;
    entity->addComponent(std::move(tourney));
    return true;
}

bool TournamentSystem::registerPlayer(const std::string& entity_id,
                                       const std::string& player_id,
                                       const std::string& player_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney || tourney->status != "registration") return false;

    if (static_cast<int>(tourney->participants.size()) >= tourney->max_participants) return false;

    // Check for duplicate registration
    for (const auto& p : tourney->participants) {
        if (p.player_id == player_id) return false;
    }

    components::Tournament::Participant participant;
    participant.player_id = player_id;
    participant.player_name = player_name;
    participant.score = 0;
    participant.kills = 0;
    participant.eliminated = false;
    tourney->participants.push_back(participant);

    tourney->prize_pool += tourney->entry_fee;
    return true;
}

bool TournamentSystem::startTournament(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney || tourney->status != "registration") return false;

    if (tourney->participants.empty()) return false;

    tourney->status = "active";
    tourney->current_round = 1;
    tourney->round_timer = tourney->round_duration;
    return true;
}

bool TournamentSystem::recordKill(const std::string& entity_id,
                                   const std::string& player_id,
                                   int points) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney || tourney->status != "active") return false;

    for (auto& p : tourney->participants) {
        if (p.player_id == player_id && !p.eliminated) {
            p.score += points;
            p.kills++;
            return true;
        }
    }
    return false;
}

bool TournamentSystem::eliminatePlayer(const std::string& entity_id,
                                        const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney || tourney->status != "active") return false;

    for (auto& p : tourney->participants) {
        if (p.player_id == player_id && !p.eliminated) {
            p.eliminated = true;
            return true;
        }
    }
    return false;
}

int TournamentSystem::getPlayerScore(const std::string& entity_id,
                                      const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return 0;

    for (const auto& p : tourney->participants) {
        if (p.player_id == player_id) return p.score;
    }
    return 0;
}

int TournamentSystem::getParticipantCount(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return 0;

    return static_cast<int>(tourney->participants.size());
}

int TournamentSystem::getActiveParticipantCount(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return 0;

    int count = 0;
    for (const auto& p : tourney->participants) {
        if (!p.eliminated) count++;
    }
    return count;
}

std::string TournamentSystem::getStatus(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return "";

    return tourney->status;
}

int TournamentSystem::getCurrentRound(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return 0;

    return tourney->current_round;
}

double TournamentSystem::getPrizePool(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0;

    auto* tourney = entity->getComponent<components::Tournament>();
    if (!tourney) return 0.0;

    return tourney->prize_pool;
}

} // namespace systems
} // namespace atlas
