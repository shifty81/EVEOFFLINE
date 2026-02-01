"""
Data Loader System
Loads game data from JSON files (inspired by Astrox Imperium's text-based approach)
"""

import json
import os
from typing import Dict, Any, List, Optional
from pathlib import Path


class DataLoader:
    """
    Loads and manages game data from JSON files
    Supports hot-reloading for modding
    """
    
    def __init__(self, data_dir: str = "data"):
        self.data_dir = Path(data_dir)
        self.ships: Dict[str, Dict[str, Any]] = {}
        self.modules: Dict[str, Dict[str, Any]] = {}
        self.skills: Dict[str, Dict[str, Any]] = {}
        self.npcs: Dict[str, Dict[str, Any]] = {}
        self.missions: Dict[str, Dict[str, Any]] = {}
        self.universe: Dict[str, Any] = {}
        
    def load_all(self):
        """Load all game data"""
        print("[DataLoader] Loading game data...")
        self.load_ships()
        self.load_modules()
        self.load_skills()
        self.load_npcs()
        self.load_missions()
        self.load_universe()
        print("[DataLoader] All game data loaded successfully")
        
    def load_ships(self):
        """Load ship definitions"""
        ships_dir = self.data_dir / "ships"
        if not ships_dir.exists():
            print(f"[DataLoader] Warning: Ships directory not found: {ships_dir}")
            return
            
        for file_path in ships_dir.glob("*.json"):
            with open(file_path, 'r') as f:
                data = json.load(f)
                # Data can be a dict of ships or a list
                if isinstance(data, dict):
                    self.ships.update(data)
                elif isinstance(data, list):
                    for ship in data:
                        self.ships[ship['id']] = ship
                        
        print(f"[DataLoader] Loaded {len(self.ships)} ships")
        
    def load_modules(self):
        """Load module definitions"""
        modules_dir = self.data_dir / "modules"
        if not modules_dir.exists():
            print(f"[DataLoader] Warning: Modules directory not found: {modules_dir}")
            return
            
        for file_path in modules_dir.glob("*.json"):
            with open(file_path, 'r') as f:
                data = json.load(f)
                if isinstance(data, dict):
                    self.modules.update(data)
                elif isinstance(data, list):
                    for module in data:
                        self.modules[module['id']] = module
                        
        print(f"[DataLoader] Loaded {len(self.modules)} modules")
        
    def load_skills(self):
        """Load skill definitions"""
        skills_file = self.data_dir / "skills" / "skills.json"
        if not skills_file.exists():
            print(f"[DataLoader] Warning: Skills file not found: {skills_file}")
            return
            
        with open(skills_file, 'r') as f:
            data = json.load(f)
            if isinstance(data, dict):
                self.skills = data
            elif isinstance(data, list):
                for skill in data:
                    self.skills[skill['id']] = skill
                    
        print(f"[DataLoader] Loaded {len(self.skills)} skills")
        
    def load_npcs(self):
        """Load NPC definitions"""
        npcs_dir = self.data_dir / "npcs"
        if not npcs_dir.exists():
            print(f"[DataLoader] Warning: NPCs directory not found: {npcs_dir}")
            return
            
        for file_path in npcs_dir.glob("*.json"):
            with open(file_path, 'r') as f:
                data = json.load(f)
                if isinstance(data, dict):
                    self.npcs.update(data)
                elif isinstance(data, list):
                    for npc in data:
                        self.npcs[npc['id']] = npc
                        
        print(f"[DataLoader] Loaded {len(self.npcs)} NPC types")
        
    def load_missions(self):
        """Load mission definitions"""
        missions_dir = self.data_dir / "missions"
        if not missions_dir.exists():
            print(f"[DataLoader] Warning: Missions directory not found: {missions_dir}")
            return
            
        for file_path in missions_dir.glob("*.json"):
            with open(file_path, 'r') as f:
                data = json.load(f)
                if isinstance(data, dict):
                    self.missions.update(data)
                elif isinstance(data, list):
                    for mission in data:
                        self.missions[mission['id']] = mission
                        
        print(f"[DataLoader] Loaded {len(self.missions)} missions")
        
    def load_universe(self):
        """Load universe structure"""
        universe_file = self.data_dir / "universe" / "systems.json"
        if not universe_file.exists():
            print(f"[DataLoader] Warning: Universe file not found: {universe_file}")
            return
            
        with open(universe_file, 'r') as f:
            self.universe = json.load(f)
            
        print(f"[DataLoader] Loaded universe with {len(self.universe.get('systems', []))} systems")
        
    def get_ship(self, ship_id: str) -> Optional[Dict[str, Any]]:
        """Get ship data by ID"""
        return self.ships.get(ship_id)
        
    def get_module(self, module_id: str) -> Optional[Dict[str, Any]]:
        """Get module data by ID"""
        return self.modules.get(module_id)
        
    def get_skill(self, skill_id: str) -> Optional[Dict[str, Any]]:
        """Get skill data by ID"""
        return self.skills.get(skill_id)
        
    def get_npc(self, npc_id: str) -> Optional[Dict[str, Any]]:
        """Get NPC data by ID"""
        return self.npcs.get(npc_id)
        
    def get_mission(self, mission_id: str) -> Optional[Dict[str, Any]]:
        """Get mission data by ID"""
        return self.missions.get(mission_id)
