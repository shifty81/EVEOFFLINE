# Standings System Documentation

## Overview

The Standings System tracks relationships between entities (players, NPCs, corporations, and factions) on a scale from -10 (terrible) to +10 (excellent). Standings affect NPC behavior, agent access, market fees, and mission rewards.

## Component Structure

### Standings Component (`components::Standings`)

Located in `cpp_server/include/components/game_components.h`

The `Standings` component contains three types of standings:

```cpp
class Standings : public ecs::Component {
public:
    std::map<std::string, float> personal_standings;     // Individual relationships
    std::map<std::string, float> corporation_standings;  // Corp-level relationships
    std::map<std::string, float> faction_standings;      // Faction-wide relationships
};
```

### Standing Priority

When determining the standing between two entities, the system checks in priority order:

1. **Personal standings** (highest priority) - Direct relationship with specific entity
2. **Corporation standings** (medium priority) - Relationship with the entity's corporation
3. **Faction standings** (lowest priority) - Relationship with the entity's faction

If no standing exists at any level, the default is **0 (neutral)**.

## Standing Scale

| Range | Label | Description |
|-------|-------|-------------|
| +10.0 | Excellent | Maximum positive standing |
| +5.0 to +10.0 | Good | Positive relationship |
| +0.1 to +5.0 | Neutral Positive | Slight positive |
| 0.0 | Neutral | No relationship |
| -0.1 to -5.0 | Neutral Negative | Slight negative |
| -5.0 to -10.0 | Bad | Hostile relationship |
| -10.0 | Terrible | Maximum negative standing |

## Default Standings

### Player Default Standings (on spawn)

```cpp
// Empire factions - neutral
standings->faction_standings["Caldari"] = 0.0f;
standings->faction_standings["Gallente"] = 0.0f;
standings->faction_standings["Amarr"] = 0.0f;
standings->faction_standings["Minmatar"] = 0.0f;

// Pirate factions - hostile
standings->faction_standings["Serpentis"] = -5.0f;
standings->faction_standings["Guristas"] = -5.0f;
standings->faction_standings["Blood Raiders"] = -5.0f;
standings->faction_standings["Sansha's Nation"] = -5.0f;
```

### NPC Default Standings

**Pirate NPCs:**
- Hostile (-5.0) to all empire factions (Caldari, Gallente, Amarr, Minmatar)

**Empire NPCs:**
- Neutral (0.0) to all empire factions

## API Methods

### `getStandingWith()`

Retrieves the standing value for a given entity with priority fallback.

```cpp
float getStandingWith(const std::string& entity_id, 
                     const std::string& entity_corp = "",
                     const std::string& entity_faction = "") const;
```

**Example:**
```cpp
auto* standings = player->getComponent<components::Standings>();
float standing = standings->getStandingWith("npc_pirate_001", "Serpentis Corp", "Serpentis");
// Returns personal standing if set, otherwise corp standing, otherwise faction standing
```

### `modifyStanding()`

Modifies a standing value with automatic clamping to valid range (-10 to +10).

```cpp
static void modifyStanding(std::map<std::string, float>& standing_map,
                          const std::string& key,
                          float change);
```

**Example:**
```cpp
auto* standings = player->getComponent<components::Standings>();
// Increase standing with a faction by 0.5
Standings::modifyStanding(standings->faction_standings, "Caldari", 0.5f);
// Decrease standing with an NPC by 2.0
Standings::modifyStanding(standings->personal_standings, "npc_001", -2.0f);
```

## Client-Side Standing Calculation

The client calculates display standings based on faction information without direct access to the full Standings component.

Located in `cpp_client/src/ui/overview_panel.cpp`:

```cpp
int OverviewPanel::CalculateStanding(const std::string& faction, bool is_player) const {
    // Players are neutral
    if (is_player) return 0;
    
    // Pirate factions are hostile
    if (faction == "Serpentis" || faction == "Guristas" || ...) return -5;
    
    // Empire factions are neutral
    if (faction == "Caldari" || faction == "Gallente" || ...) return 0;
    
    // Friendly factions
    if (faction == "CONCORD" || faction == "ORE" || ...) return 5;
    
    return 0; // Default neutral
}
```

## Standing Effects

### Current Implementation

**Overview Panel:**
- Color-coding based on standing (red = hostile, yellow = neutral, green = friendly)
- Standing value displayed in dedicated column
- Filter options: show hostile/neutral/friendly

### Future Implementation (Planned)

**Agent Access:**
- Level 1 agents: No standing requirement
- Level 2 agents: +1.0 standing required
- Level 3 agents: +3.0 standing required
- Level 4 agents: +5.0 standing required
- Level 5 agents: +7.0 standing required

**Market Fees:**
- Broker fees reduced by 1% per +1.0 standing (max 5% reduction)
- Increased by 1% per -1.0 standing (max 5% increase)

**NPC Behavior:**
- Standings < -2.0: NPCs may attack on sight
- Standings < -5.0: Faction police engage in high-sec
- Standings > 5.0: NPCs provide assistance when attacked

**Mission Rewards:**
- ISK rewards increased by 5% per +1.0 standing (max 50%)
- LP gains increased by 10% per +1.0 standing (max 100%)

## Serialization

Standings are automatically saved/loaded with world persistence:

```json
{
  "standings": {
    "personal": {
      "npc_pirate_001": -5.0,
      "player_friend": 8.5
    },
    "corporation": {
      "Republic Fleet": 3.0,
      "Serpentis": -7.5
    },
    "faction": {
      "Minmatar": 2.5,
      "Amarr": -1.5
    }
  }
}
```

## Example Scenarios

### Scenario 1: Mission Reward
```cpp
// Player completes a mission for Caldari Navy
auto* standings = player->getComponent<components::Standings>();
Standings::modifyStanding(standings->corporation_standings, "Caldari Navy", 0.05f);
Standings::modifyStanding(standings->faction_standings, "Caldari", 0.02f);
```

### Scenario 2: Combat Penalties
```cpp
// Player attacks a Gallente NPC
auto* standings = player->getComponent<components::Standings>();
Standings::modifyStanding(standings->corporation_standings, "Gallente Federation", -0.1f);
Standings::modifyStanding(standings->faction_standings, "Gallente", -0.05f);
```

### Scenario 3: Personal Contact
```cpp
// Player sets a personal contact
auto* standings = player->getComponent<components::Standings>();
standings->personal_standings["player_ally"] = 10.0f;  // Excellent standing
standings->personal_standings["player_enemy"] = -10.0f;  // Terrible standing
```

## Testing

Comprehensive tests are located in `cpp_server/test_systems.cpp`:

- **testSerializeDeserializeStandings**: Tests JSON serialization/deserialization
- **testStandingsGetStanding**: Tests priority hierarchy (personal > corp > faction)
- **testStandingsModify**: Tests modification and clamping behavior

Run tests:
```bash
cd cpp_server/build/bin
./test_systems
```

## Integration Points

### Server
- `GameSession::createPlayerEntity()` - Initializes player standings
- `GameSession::spawnNPC()` - Initializes NPC standings
- `WorldPersistence` - Saves/loads standings data

### Client
- `OverviewPanel::UpdateEntries()` - Uses standings for color-coding
- `OverviewPanel::CalculateStanding()` - Determines standing from faction
- `OverviewPanel::GetStandingColor()` - Maps standing to display color

## Future Enhancements

1. **Standings Decay**: Automatic decay toward neutral over time
2. **Social Skills**: Skills that affect standing gains (Diplomacy, Connections)
3. **Derived Standings**: Calculate derived standings from related entities
4. **Standings Notifications**: Alert players when standings change
5. **Standings Management UI**: Client panel for viewing/managing contacts
6. **Corporation Standings**: Track corp-to-corp standings separately
7. **War Declarations**: Override standings during wars

## References

- EVE Online Manual: Chapter 11 (Missions & Agents)
- `docs/design/DESIGN.md`: Section on missions and standings
- `docs/NEXT_TASKS.md`: Standings system expansion task
- `data/corporations/corporations.json`: Corporation standings configuration
