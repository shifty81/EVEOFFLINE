# EVE Online Docking Mechanics and Station Interface

## Overview

When docked at a station in EVE Online, whether an NPC-operated station or a player-owned Upwell structure (Citadel, Engineering Complex, Refinery), the game transitions from a "3D space" environment to a "3D hangar" scene. The scene emphasizes the massive scale of ships against the backdrop of industrial, faction-specific, or player-managed infrastructure.

## 1. The Docked Scene (Visual Environment)

### The Hangar View

**Basic Scene:**
- Your active ship is parked in a large docking bay
- Ship is attached to docking cranes
- Surrounded by scaffolding and dangling cables
- Often shows other ships of similar size in the background

**Camera Controls:**
- Freely rotate camera around your ship to inspect it
- Zoom in to see ship details
- Zoom out to see the full hangar environment

**Animated Elements:**
- Animated cranes moving and adjusting
- Cargo bays that adjust to ship size
- Worker NPCs or drones moving in the background
- Blinking lights and industrial activity

### Visual Distinctions: NPC Stations

**Faction-Specific Design:**

NPC stations are highly detailed and designed around specific NPC factions:

**Amarr Stations:**
- Golden, cathedral-like architecture
- Ornate design with religious symbolism
- Grand scale with elegant curves
- Well-lit with warm, golden lighting
- Luxurious and imposing feel

**Caldari Stations:**
- Utilitarian, angular design
- Steel blue, gray, and white colors
- Functional efficiency focus
- Minimalist aesthetic
- Stark, practical lighting
- Clean, corporate feel

**Gallente Stations:**
- Organic, flowing curves
- Green, white, and silver colors
- Eco-friendly design elements
- Nature integration where possible
- Luxurious interior spaces
- Bright, welcoming atmosphere

**Minmatar Stations:**
- Rusted, patchwork appearance
- Asymmetric design
- Salvaged parts and rough edges
- Rust brown, orange, and dark gray
- Industrial, utilitarian feel
- Dim, functional lighting

**Atmosphere:**
- NPC stations feel "lived-in" and bustling
- Animated and illuminated dock structures
- Rich environmental details
- Faction identity clearly visible

### Visual Distinctions: Player-Owned Structures (Upwell)

**Upwell Structure Hangars:**
- More industrial and modular design
- Generally less "cluttered" than older NPC stations
- Massive bays designed to accommodate various ship sizes
- Modern, streamlined aesthetic
- Player customization elements (corp logos, colors)

**Structure Types:**

**Citadels (Astrahus, Fortizar, Keepstar):**
- Primary use: Command centers, staging areas
- Large open hangars with modular design
- Defensive installations visible

**Engineering Complexes (Raitaru, Azbel, Sotiyo):**
- Industrial focus with manufacturing equipment
- Assembly lines and construction facilities visible
- More utilitarian appearance

**Refineries (Athanor, Tatara):**
- Mining and ore processing equipment
- Smelters and refining machinery visible
- Resource storage containers

### Supercapitals and Titans

**Special Docking Requirements:**
- Supercapitals (Supercarriers, Titans, Force Auxiliaries) can **only dock** in **Keepstar-class** structures
- Keepstars feature unique, immense docking bays specifically designed for these massive vessels

**Keepstar Hangar Features:**
- Dramatically larger scale than other stations
- Multiple levels and platforms
- Massive support structures and gantries
- Impressive sense of scale showcasing ship size

## 2. The Docked GUI (Interface)

Upon docking, the main screen changes from the tactical space view to a menu-driven interface anchored by two primary elements:

### The Neocom (Left Side Menu)

**Description:**
- Primary navigation menu on the left side of screen
- Icon-based vertical menu bar
- Access to all major game functions

**Key Sections:**
- **Character Sheet**: Skills, attributes, biography
- **Inventory**: All items, ships, containers
- **Market**: Trading interface
- **Industry**: Manufacturing, research
- **Corporation**: Corp management
- **Mail**: In-game email
- **Chat**: Channels and conversations
- **Fleet**: Fleet management
- **Map**: Star map and navigation

### Station Services Panel (Right Side)

**Location**: Right side of screen when docked

**Available Services:**

**Industry:**
- Launch manufacturing jobs
- Blueprint research and copying
- Invention activities
- Often lower taxes in player structures vs NPC stations

**Market:**
- Access to local market orders
- Regional market browser
- Structure-specific trading (in player structures)
- Buy and sell orders
- Quick trade functionality

**Reprocessing:**
- Refine ore into minerals
- Reprocess ships and modules
- Efficiency based on skills and station equipment
- Tax rates vary by station/structure

**Ship Hangar:**
- Access personal ship collection
- Switch between available ships
- Board different ships
- View ship statistics

**Item Hangar:**
- Access personal item storage
- Organize inventory
- Move items between cargo and hangar
- Manage containers

**Corporate Hangar:**
- Access corporation-shared items
- Requires appropriate corp roles/permissions
- Multiple divisions (usually 7)
- Shared storage for corp members

**Repair Shop:**
- Repair ship damage
- Repackage modules
- Insurance services

**Fitting Service:**
- Modify ship fittings
- Add/remove modules
- Save and load fitting templates
- View fitting requirements (CPU, PowerGrid)

### Top Bar Information

**Displayed Information:**
- Current ship name and type
- Corporation name and logo
- Alliance name (if applicable)
- Current location (station/structure name)
- ISK wallet balance
- Skill queue status

### Structure Browser/Info Panel

**In Player-Owned Structures:**
- View if structure is properly anchored
- Check fuel status and remaining time
- See tax rates for services
- View access lists and permissions
- Check structure services available
- View structure owner and corporation

### Undock Button

**Location**: Prominently displayed, usually on right side

**Functionality:**
- Leave the station and enter space
- Switches back to 3D space view
- May have undocking animation/delay
- Warning if undocking into hostile environment

## 3. Key Differences: NPC vs. Player-Owned Structures

| Feature | NPC Owned Station | Player Owned (Upwell) Structure |
|---------|-------------------|----------------------------------|
| **Safety** | Absolute. Items cannot be lost. | Generally safe, but structure can be destroyed. |
| **Asset Security** | N/A - Always safe | "Asset Safety" - assets moved to nearest low-sec if destroyed |
| **Access** | Open to all (except war targets) | Restricted by owner (Access Lists, ACLs) |
| **Services** | Fixed, usually higher taxes | Modular, often better bonuses/lower taxes |
| **Service Taxes** | Standard NPC taxes | Set by structure owner, can be very low or high |
| **Market** | Regional NPC market | Structure-specific market (may be thin) |
| **Capital Docking** | Limited to Carriers/Dreads/FAX | Up to Titans (if Keepstar) |
| **Customization** | None | Owner can customize services, taxes, access |
| **Destruction Risk** | Cannot be destroyed | Can be destroyed in war/attack |
| **Fuel Requirement** | No | Yes - structure needs fuel to operate |

## 4. Special Situations

### Wormhole Space Docking

**Key Differences:**
- Player structures are the **only way to dock** in wormhole space (no NPC stations)
- If structure is destroyed, assets are **dropped as loot** rather than asset safety (for C4-C6)
- C1-C3 wormholes (as of 2026) have partial asset safety
- High-risk, high-stakes environment

**Precautions:**
- Always have an exit strategy
- Don't store irreplaceable items
- Be aware of structure status and fuel
- Monitor for hostile activity

### Player-Owned Structure (POS) - Old Type

**Description:**
- Forcefield-protected "blue bubble" moon mining bases
- Not full hangars like modern Upwell structures
- Being phased out in favor of Upwell structures

**Mechanics:**
- You "park" inside the shield bubble
- GUI acts similarly to a station while inside
- Access requires corp permissions
- Much older technology, less secure

### Access Denial and Lockouts

**If Structure Becomes Hostile:**
- You can be **locked out** if access permissions change
- Cannot dock again until permissions restored
- Items inside become inaccessible

**If Structure is Destroyed:**
- **High-Sec/Low-Sec/Null-Sec**: Assets moved to asset safety
  - Must recover from nearest low-sec NPC station
  - 15% recovery fee on asset value
  - 20 days to recover, or items moved to nearest NPC station
- **Wormhole Space (C4-C6)**: Assets **dropped as loot**
  - Complete loss if not recovered
  - Can be looted by attackers
- **Wormhole Space (C1-C3, as of 2026)**: Partial asset safety available

**Warning Signs:**
- Structure low on fuel
- Structure under attack
- Timer notifications
- Corp/alliance announcements

## 5. User Experience Flow

### Docking Sequence

1. **Approach Station**: Get within docking range (usually automatic)
2. **Docking Request**: Right-click station > "Dock" or use shortcut
3. **Permission Check**: System verifies access permissions
4. **Docking Animation**: Ship approaches and enters station
5. **Transition**: Screen transitions from space to hangar view
6. **Hangar Scene**: 3D hangar loads with ship displayed
7. **GUI Appears**: Neocom and Station Services become available

### Undocking Sequence

1. **Click Undock Button**: Initiates undocking process
2. **Undocking Delay**: Brief countdown (prevents instant redocking)
3. **Loading**: Space environment loads
4. **Exit Animation**: Ship exits station/structure
5. **Invulnerability**: Brief period of invulnerability (30 seconds, unless you move or activate modules)
6. **Space View**: Returns to full 3D space view with tactical overlay

## 6. Quality of Life Features

**Quick Actions:**
- Drag-and-drop module fitting
- Batch reprocessing
- Quick market searches from inventory
- One-click repair all
- Saved fitting loadouts

**Station Favorites:**
- Bookmark favorite stations
- Quick navigation to preferred trading hubs
- Recent locations list

**Safety Checks:**
- Warnings before undocking with valuable cargo
- Alert if station under attack
- Notification of hostile players in local

## Related Resources

- See `STRUCTURES_2026.md` for information about player-owned structures
- See `data/universe/station_types.json` for NPC station types
- See `data/universe/player_structures.json` for player structure data

---

*This document describes the docking mechanics and interface in EVE Online, providing a comprehensive reference for station and structure interaction.*
