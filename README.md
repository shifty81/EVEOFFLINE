# EVE OFFLINE

A PVE-focused space MMO inspired by EVE ONLINE, designed for small groups of players (2-20). Built with a custom Python game engine and inspired by both EVE ONLINE and Astrox Imperium.

## 🎮 Features

- **Custom Game Engine**: Built from scratch using Python with Entity Component System (ECS) architecture
- **Multiplayer PVE**: Server-authoritative multiplayer for cooperative gameplay without PVP
- **EVE-Like Mechanics**: Ships, skills, fitting, combat, missions, and exploration
- **Fully Moddable**: All game content defined in editable JSON files (ships, modules, skills, NPCs, missions)
- **Text-Based Data**: Inspired by Astrox Imperium's modding approach - edit with any text editor
- **Server-Client Architecture**: Dedicated server for persistent worlds

## 🚀 Quick Start

### Prerequisites
- Python 3.11 or higher

### Installation

```bash
# Clone the repository
git clone https://github.com/shifty81/EVEOFFLINE.git
cd EVEOFFLINE

# Optional: Create virtual environment
python -m venv venv
source venv/bin/activate  # On Windows: venv\Scripts\activate

# Install dependencies (optional for enhanced features)
pip install -r requirements.txt
```

### Running the Game

**🎨 NEW! GUI Demo (Visual gameplay - Recommended to see visuals!):**
```bash
python gui_demo.py
```
Graphical demo with pygame showing:
- 2D space visualization with star field
- Visual ship representations with health bars
- Real-time combat with weapon effects
- Interactive camera controls (pan, zoom)
- **No server required** - runs standalone!

**🎮 Interactive Gameplay Demo (Text-based, feature-complete):**
```bash
python interactive_demo.py
```
This provides a menu-driven interface to explore all game features:
- Ship status and fitting
- Drone operations
- Combat and targeting
- Skills and progression
- Navigation and warp
- Missions and scanning

**📺 Automated Gameplay Showcase:**
```bash
python showcase_gameplay.py
```
Demonstrates all features in a scripted format (great for screenshots/videos).

**🌐 Multiplayer Server Mode:**

**Start the server:**
```bash
python server/server.py
```

**Start a text client (in a new terminal):**
```bash
python client/client.py "YourCharacterName"
```

**Or start a GUI client (with visuals):**
```bash
python client/gui_client.py "YourCharacterName"
```

**Start multiple clients for multiplayer:**
```bash
python client/client.py "Pilot1"
python client/gui_client.py "Pilot2"
python client/client.py "Pilot3"
```

## 🧪 Testing

Run the comprehensive test suite:
```bash
python run_tests.py
```

Or run individual test files:
```bash
python tests/test_engine.py              # Core engine tests
python tests/test_exploration_angle.py   # Exploration scanning tests
python tests/test_manufacturing.py       # Manufacturing/industry tests
python tests/test_market.py              # Market/economy tests
```

See [docs/testing/TEST_SUMMARY.md](docs/testing/TEST_SUMMARY.md) for detailed test coverage information.

## 📚 Documentation

**All documentation is now organized in the [docs/](docs/) folder!**

### Quick Links
- 📋 [**Project Roadmap**](docs/ROADMAP.md) - What's done and what's planned
- 🚀 [**Getting Started Guide**](docs/getting-started/GETTING_STARTED.md) - Installation and quick start
- 📖 [**Content Guide**](docs/guides/CONTENT_GUIDE.md) - Complete guide to ships, modules, and content
- 👨‍💻 [**Developer Documentation**](docs/development/DOCUMENTATION.md) - API reference and development guide
- 🎨 [**Design Document**](docs/design/DESIGN.md) - Game design and EVE mechanics
- ✨ [**Feature Documentation**](docs/features/NEW_FEATURES.md) - Complete feature list
- 🧪 [**Test Coverage**](docs/testing/TEST_SUMMARY.md) - Testing documentation

**For a complete documentation index, see [docs/README.md](docs/README.md)**

## 🎯 Current Status

### ✅ Implemented (Phase 1 - Complete!)
- [x] Custom ECS (Entity Component System) engine
- [x] Game components (Position, Health, Ship, Weapons, etc.)
- [x] Game systems (Movement, Combat, AI, Targeting)
- [x] Data-driven architecture with JSON files
- [x] Extensive game data (14 ships, 70 modules, 47 skills, 13 NPCs, 15 missions)
- [x] Network protocol and messaging
- [x] Dedicated game server
- [x] Basic game client
- [x] Server-client state synchronization

### 🚀 New in Extended Content Pack
- [x] **10 new ships**: Destroyers and Cruisers across all 4 races
  - 4 Tech I Destroyers (Thrasher, Cormorant, Catalyst, Coercer)
  - 6 Tech I Cruisers (Stabber, Caracal, Vexor, Maller, Rupture, Moa)
- [x] **61 new modules**: Expanded from 9 to 70 modules total
  - 10 new weapons (medium turrets, blasters, railguns, beam lasers)
  - 18 defensive modules (shield extenders, armor repairers, hardeners)
  - 18 utility modules (EWAR, propulsion, tracking, engineering)
  - 14 drones (light, medium, heavy combat drones + utility drones)
- [x] **32 new skills**: Expanded from 15 to 47 skills
  - Complete drone skill tree (6 skills)
  - Medium weapon skills for all types
  - Destroyer and Cruiser piloting for all races
  - Electronic warfare and engineering skills
- [x] **9 new NPCs**: Added Sansha's Nation, Angel Cartel, and Rogue Drones
  - Destroyer and Cruiser class NPCs for existing factions
- [x] **10 new missions**: Level 2-4 missions with varied objectives
  - Combat, exploration, courier, and mining missions

### 🎯 NEW! Full EVE Experience Features (Phase 2 - Complete!)
- [x] **Module Fitting System** - CPU/PowerGrid management, slot validation
- [x] **Drone System** - Launch, recall, engage, bandwidth management
- [x] **Skill Training** - Queue system, SP accumulation, skill bonuses
- [x] **Mission System** - Accept, track, complete missions with rewards
- [x] **Navigation & Warp** - FTL travel, alignment, warp mechanics
- [x] **Docking System** - Station docking/undocking with services
- [x] **Stargate Jumps** - System-to-system travel
- [x] **Advanced Movement** - Approach and orbit commands
- [x] **Combat Enhancements** - Full resistance system (EM/Thermal/Kinetic/Explosive)
- [x] **10 New Components** - Module, Drone, DroneBay, Mission, WarpDrive, and more

### 🎨 NEW! Visual/GUI Features
- [x] **Pygame-based GUI** - 2D graphical rendering system
- [x] **Visual Space Representation** - Star field, ships, and entities
- [x] **Health Bar Overlays** - Shield/Armor/Hull visualization
- [x] **Weapon Effects** - Visual feedback for combat
- [x] **Interactive Camera** - Pan, zoom, and follow controls
- [x] **HUD/UI Overlay** - Status information and combat log
- [x] **Standalone GUI Demo** - No server required to see visuals
- [x] **GUI Multiplayer Client** - Connect to server with graphics

**See [docs/features/NEW_FEATURES.md](docs/features/NEW_FEATURES.md) for complete feature documentation!**
**See [docs/features/VISUAL_CAPABILITIES.md](docs/features/VISUAL_CAPABILITIES.md) for GUI/visual documentation!**

### 🎯 NEW! Phase 3 Features - Complete!
- [x] **Manufacturing System** - Blueprint research, manufacturing, copying
- [x] **Market System** - Buy/sell orders, instant trading, ISK wallet
- [x] **Exploration System** - Probe scanning, cosmic signatures, sites
- [x] **Loot System** - NPC drops, loot containers, salvaging
- [x] **Fleet System** - Fleet bonuses, organization, coordination
- [x] **Inventory System** - Item storage with capacity management

### 🎯 NEW! Phase 4 Features - Complete!
- [x] **Corporation System** - Full corp management, roles, permissions
- [x] **Corporation Hangars** - Shared storage with 3 divisions
- [x] **Corporation Wallets** - Shared ISK pool with tax system
- [x] **Social Features** - Contacts, standings, blocking
- [x] **Mail System** - Send/receive mail, inbox/sent folders
- [x] **Chat System** - Channels, moderation, chat history
- [x] **Contract System** - Item exchange, courier contracts

### 🚀 Next Up: Phase 5
- [ ] 3D rendering (see [docs/features/LANGUAGE_AND_3D_OPTIONS.md](docs/features/LANGUAGE_AND_3D_OPTIONS.md) for options)
- [ ] Advanced UI enhancements
- [ ] More content (ships, modules, missions)

**See [docs/ROADMAP.md](docs/ROADMAP.md) for complete status and planned features!**

## 🎮 Game Mechanics

Based on EVE ONLINE with simplified mechanics for small-group PVE:

- **Ships**: Frigates, Cruisers, Battleships with fitting slots
- **Combat**: Shield/Armor/Hull system with damage types and resistances
- **Skills**: Time-based training (accelerated compared to EVE)
- **Missions**: Combat, Mining, Courier, and Exploration missions
- **NPCs**: Serpentis, Guristas, Blood Raiders with AI behaviors
- **Universe**: Multiple solar systems connected by stargates

## 🔧 Modding

All game content is moddable via JSON files in the `data/` directory:

```
data/
├── ships/          # Ship definitions
├── modules/        # Module definitions  
├── skills/         # Skill definitions
├── npcs/           # NPC definitions
├── missions/       # Mission templates
└── universe/       # Solar system data
```

See [docs/development/DOCUMENTATION.md](docs/development/DOCUMENTATION.md) for modding guides.

## 🏗️ Architecture

**Engine**: Custom Python ECS engine
**Networking**: asyncio with TCP sockets
**Data Format**: JSON for all game data
**Persistence**: SQLite (planned)

**Inspired by:**
- **EVE ONLINE** by CCP Games (game mechanics)
- **Astrox Imperium** by Jace Masula (modding approach, data-driven design)

## 🤝 Contributing

Contributions are welcome! Please read the documentation and feel free to:
- Add new ships, modules, or missions (via JSON)
- Improve game systems
- Add features
- Fix bugs
- Improve documentation

## 📝 License

[To be determined]

## 🎯 Project Goals

1. Recreate the EVE ONLINE experience for small groups
2. Focus on PVE content without PVP stress
3. Fully moddable and community-driven
4. Accessible and fun for casual players
5. Built with modern software architecture

## 📧 Contact

[Project information to be added]

---

**Note**: This is an indie project inspired by EVE ONLINE. It is not affiliated with or endorsed by CCP Games.
