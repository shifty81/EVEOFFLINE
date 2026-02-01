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

**Start the server:**
```bash
python server/server.py
```

**Start a client (in a new terminal):**
```bash
python client/client.py "YourCharacterName"
```

**Start multiple clients for multiplayer:**
```bash
python client/client.py "Pilot1"
python client/client.py "Pilot2"
python client/client.py "Pilot3"
```

## 📚 Documentation

- [DESIGN.md](DESIGN.md) - Game design document with EVE ONLINE mechanics
- [ASTROX_RESEARCH.md](ASTROX_RESEARCH.md) - Research on Astrox Imperium's approach
- [DOCUMENTATION.md](DOCUMENTATION.md) - Developer documentation and API reference

## 🎯 Current Status

### ✅ Implemented (Phase 1)
- [x] Custom ECS (Entity Component System) engine
- [x] Game components (Position, Health, Ship, Weapons, etc.)
- [x] Game systems (Movement, Combat, AI, Targeting)
- [x] Data-driven architecture with JSON files
- [x] Sample game data (ships, modules, skills, NPCs, missions)
- [x] Network protocol and messaging
- [x] Dedicated game server
- [x] Basic game client
- [x] Server-client state synchronization

### 🔨 In Progress (Phase 2)
- [ ] Combat system implementation
- [ ] NPC AI behaviors
- [ ] Mission system
- [ ] Skills and progression
- [ ] Module fitting system

### 📋 Planned (Phase 3+)
- [ ] 2D/3D rendering (Pygame or Pyglet)
- [ ] User interface
- [ ] More ships and modules
- [ ] Mining and resource gathering
- [ ] Economy system
- [ ] Fleet mechanics
- [ ] Modding tools

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

See [DOCUMENTATION.md](DOCUMENTATION.md) for modding guides.

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
