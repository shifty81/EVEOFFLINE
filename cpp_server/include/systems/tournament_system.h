#ifndef EVE_SYSTEMS_TOURNAMENT_SYSTEM_H
#define EVE_SYSTEMS_TOURNAMENT_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages competitive PvE tournament events
 *
 * Handles tournament lifecycle: creation, registration, round
 * progression, scoring, elimination, and prize distribution.
 */
class TournamentSystem : public ecs::System {
public:
    explicit TournamentSystem(ecs::World* world);
    ~TournamentSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "TournamentSystem"; }

    /**
     * @brief Create a new tournament
     * @return true if tournament was created
     */
    bool createTournament(const std::string& entity_id,
                          const std::string& tournament_id,
                          const std::string& name,
                          int max_participants,
                          double entry_fee,
                          float round_duration);

    /**
     * @brief Register a player for a tournament
     * @return true if registration succeeded
     */
    bool registerPlayer(const std::string& entity_id,
                        const std::string& player_id,
                        const std::string& player_name);

    /**
     * @brief Start the tournament (move from registration to active)
     * @return true if tournament started
     */
    bool startTournament(const std::string& entity_id);

    /**
     * @brief Record a kill score for a participant
     * @return true if score was recorded
     */
    bool recordKill(const std::string& entity_id,
                    const std::string& player_id,
                    int points = 1);

    /**
     * @brief Eliminate a participant from the tournament
     * @return true if participant was eliminated
     */
    bool eliminatePlayer(const std::string& entity_id,
                         const std::string& player_id);

    /**
     * @brief Get a participant's score
     */
    int getPlayerScore(const std::string& entity_id,
                       const std::string& player_id);

    /**
     * @brief Get the number of registered participants
     */
    int getParticipantCount(const std::string& entity_id);

    /**
     * @brief Get the number of active (non-eliminated) participants
     */
    int getActiveParticipantCount(const std::string& entity_id);

    /**
     * @brief Get tournament status
     */
    std::string getStatus(const std::string& entity_id);

    /**
     * @brief Get current round number
     */
    int getCurrentRound(const std::string& entity_id);

    /**
     * @brief Get the prize pool
     */
    double getPrizePool(const std::string& entity_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_TOURNAMENT_SYSTEM_H
