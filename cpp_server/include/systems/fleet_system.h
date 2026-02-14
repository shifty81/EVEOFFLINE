#ifndef EVE_SYSTEMS_FLEET_SYSTEM_H
#define EVE_SYSTEMS_FLEET_SYSTEM_H

#include "ecs/system.h"
#include <string>
#include <vector>
#include <map>

namespace atlas {
namespace systems {

/**
 * @brief Fleet member data tracked by the FleetSystem
 */
struct FleetMemberInfo {
    std::string entity_id;
    std::string character_name;
    std::string role = "Member";  // FleetCommander, WingCommander, SquadCommander, Member
    std::string squad_id;
    std::string wing_id;
    bool online = true;
};

/**
 * @brief Fleet bonus definition
 */
struct FleetBonus {
    std::string type;    // "armor", "shield", "skirmish", "information"
    std::string stat;    // e.g. "hp_bonus", "resist_bonus", "speed_bonus"
    float value = 0.0f;  // multiplier (e.g. 0.10 = +10%)
};

/**
 * @brief A fleet instance containing its members and organization
 */
struct Fleet {
    std::string fleet_id;
    std::string fleet_name;
    std::string commander_entity_id;
    std::map<std::string, FleetMemberInfo> members;  // entity_id -> info
    std::map<std::string, std::vector<std::string>> squads;  // squad_id -> [entity_ids]
    std::map<std::string, std::vector<std::string>> wings;   // wing_id -> [squad_ids]
    std::map<std::string, std::string> active_boosters;  // booster_type -> entity_id
    size_t max_members = 256;
    bool player_fleet = false;  // true = capped at 5 members (player + 4 captains)
};

/**
 * @brief Manages fleet creation, membership, bonuses, and coordination
 *
 * Implements EVE Online-style fleet mechanics including hierarchical
 * organization (Fleet -> Wings -> Squads), role-based permissions,
 * fleet bonuses, target broadcasting, and fleet warp commands.
 */
class FleetSystem : public ecs::System {
public:
    explicit FleetSystem(ecs::World* world);
    ~FleetSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetSystem"; }

    // --- Fleet lifecycle ---

    /**
     * @brief Create a new fleet with the given entity as fleet commander
     * @return fleet_id on success, empty string on failure
     */
    std::string createFleet(const std::string& commander_entity_id,
                            const std::string& fleet_name = "Fleet");

    /**
     * @brief Disband a fleet (FC only)
     * @return true if disbanded
     */
    bool disbandFleet(const std::string& fleet_id, const std::string& requester_entity_id);

    // --- Membership ---

    /**
     * @brief Invite / add a member to a fleet
     * @return true if added
     */
    bool addMember(const std::string& fleet_id, const std::string& entity_id,
                   const std::string& character_name = "");

    /**
     * @brief Remove a member (or leave voluntarily)
     * @return true if removed
     */
    bool removeMember(const std::string& fleet_id, const std::string& entity_id);

    /**
     * @brief Get the fleet a given entity belongs to
     * @return fleet_id or empty string
     */
    std::string getFleetForEntity(const std::string& entity_id) const;

    // --- Roles ---

    /**
     * @brief Promote a member to a new role (FC only)
     */
    bool promoteMember(const std::string& fleet_id,
                       const std::string& requester_entity_id,
                       const std::string& target_entity_id,
                       const std::string& new_role);

    // --- Organization ---

    /**
     * @brief Assign a member to a squad
     */
    bool assignToSquad(const std::string& fleet_id,
                       const std::string& entity_id,
                       const std::string& squad_id);

    /**
     * @brief Assign a squad to a wing
     */
    bool assignSquadToWing(const std::string& fleet_id,
                           const std::string& squad_id,
                           const std::string& wing_id);

    // --- Bonuses ---

    /**
     * @brief Set a fleet booster for a bonus type
     * @param booster_type "armor", "shield", "skirmish", or "information"
     */
    bool setBooster(const std::string& fleet_id,
                    const std::string& booster_type,
                    const std::string& booster_entity_id);

    /**
     * @brief Get the active bonuses for a booster type
     */
    std::vector<FleetBonus> getBonusesForType(const std::string& booster_type) const;

    // --- Coordination ---

    /**
     * @brief Broadcast a target to all fleet members
     * @return number of members notified
     */
    int broadcastTarget(const std::string& fleet_id,
                        const std::string& broadcaster_entity_id,
                        const std::string& target_entity_id);

    /**
     * @brief Initiate fleet warp (FC / Wing Commander only)
     * @return number of members warped
     */
    int fleetWarp(const std::string& fleet_id,
                  const std::string& commander_entity_id,
                  float dest_x, float dest_y, float dest_z);

    // --- Queries ---

    /**
     * @brief Get the Fleet object (read-only)
     * @return pointer to fleet or nullptr
     */
    const Fleet* getFleet(const std::string& fleet_id) const;

    /**
     * @brief Get count of active fleets
     */
    size_t getFleetCount() const;

    /**
     * @brief Get member count for a fleet
     */
    size_t getMemberCount(const std::string& fleet_id) const;

    // --- Player Fleet (player + up to 4 AI captains) ---

    /**
     * @brief Create a player fleet capped at 5 ships (1 player + 4 captains)
     * @param player_entity_id The player's ship entity
     * @param fleet_name Display name
     * @return fleet_id on success, empty string on failure
     */
    std::string createPlayerFleet(const std::string& player_entity_id,
                                  const std::string& fleet_name = "Player Fleet");

    /**
     * @brief Assign an AI captain to the player fleet
     * @param fleet_id Player fleet ID
     * @param captain_entity_id AI captain ship entity
     * @param captain_name Display name
     * @return true if captain was added (fleet must have room, max 4 captains)
     */
    bool assignCaptain(const std::string& fleet_id,
                       const std::string& captain_entity_id,
                       const std::string& captain_name = "Captain");

    /**
     * @brief Check whether a fleet is a player fleet (5-ship cap)
     */
    bool isPlayerFleet(const std::string& fleet_id) const;

    static constexpr size_t PLAYER_FLEET_MAX = 5;

private:
    std::map<std::string, Fleet> fleets_;       // fleet_id -> Fleet
    std::map<std::string, std::string> entity_fleet_;  // entity_id -> fleet_id
    int next_fleet_id_ = 1;

    void applyFleetBonuses(const std::string& fleet_id);
    void removeFleetBonuses(const std::string& entity_id);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_FLEET_SYSTEM_H
