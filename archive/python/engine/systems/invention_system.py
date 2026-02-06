"""
Research & Invention System for EVE OFFLINE

Implements Tech II blueprint invention mechanics based on EVE Online:
- Invent T2 BPCs from T1 BPCs
- Consumes datacores and optional decryptors
- Success chance based on skills and decryptors
- Resulting T2 BPCs have limited runs
"""

import random
from typing import Dict, List, Optional, Tuple
from engine.core.ecs import Entity
from ..components.game_components import Inventory, SkillTraining


class InventionSystem:
    """System for Tech II blueprint invention"""
    
    # Base invention success rates by item type
    BASE_SUCCESS_RATES = {
        "module": 0.34,      # 34% base for modules
        "frigate": 0.30,     # 30% base for frigates
        "destroyer": 0.30,   # 30% base for destroyers
        "cruiser": 0.25,     # 25% base for cruisers
        "battlecruiser": 0.20,  # 20% base for battlecruisers
        "battleship": 0.18,  # 18% base for battleships
    }
    
    # Skill multiplier per level (5% per level of each science skill)
    SKILL_MULTIPLIER_PER_LEVEL = 0.01  # 1% per skill level
    
    # Default T2 BPC runs (can be modified by decryptors)
    DEFAULT_T2_RUNS = {
        "module": 10,
        "frigate": 1,
        "destroyer": 1,
        "cruiser": 1,
        "battlecruiser": 1,
        "battleship": 1,
    }
    
    def __init__(self, world):
        self.world = world
    
    def can_invent(self, entity: 'Entity', t1_bpc_id: str, 
                   datacore1_id: str, datacore2_id: str,
                   decryptor_id: Optional[str] = None) -> Tuple[bool, str]:
        """
        Check if entity can perform invention
        
        Returns:
            Tuple[bool, str]: (can_invent, error_message)
        """
        # Get entity components
        inventory = entity.components.get(Inventory)
        if not inventory:
            return False, "No inventory component"
        
        # Check T1 BPC exists
        if t1_bpc_id not in inventory.items:
            return False, f"T1 BPC {t1_bpc_id} not found in inventory"
        
        t1_bpc = inventory.items[t1_bpc_id]
        if not isinstance(t1_bpc, dict) or t1_bpc.get("type") != "blueprint":
            return False, f"Item {t1_bpc_id} is not a blueprint"
        
        if t1_bpc.get("tech_level", 1) != 1:
            return False, "Can only invent from Tech I blueprints"
        
        if t1_bpc.get("is_original", False):
            return False, "Cannot invent from BPO, must use BPC"
        
        # Check datacores exist
        for datacore_id in [datacore1_id, datacore2_id]:
            if datacore_id not in inventory.items:
                return False, f"Datacore {datacore_id} not found"
            
            datacore = inventory.items[datacore_id]
            if not isinstance(datacore, dict) or datacore.get("type") != "datacore":
                return False, f"Item {datacore_id} is not a datacore"
        
        # Check decryptor if provided
        if decryptor_id:
            if decryptor_id not in inventory.items:
                return False, f"Decryptor {decryptor_id} not found"
            
            decryptor = inventory.items[decryptor_id]
            if not isinstance(decryptor, dict) or decryptor.get("type") != "decryptor":
                return False, f"Item {decryptor_id} is not a decryptor"
        
        # Check required skills
        skills = entity.components.get(SkillTraining)
        if not skills:
            return False, "No skill training component"
        
        required_skills = t1_bpc.get("invention_skills", [])
        for skill_id in required_skills:
            if skill_id not in skills.trained_skills:
                return False, f"Missing required skill: {skill_id}"
            if skills.trained_skills[skill_id] < 1:
                return False, f"Skill {skill_id} must be at least level 1"
        
        return True, "Can invent"
    
    def calculate_success_chance(self, entity: 'Entity', t1_bpc_id: str,
                                 decryptor_id: Optional[str] = None) -> float:
        """
        Calculate invention success chance
        
        Formula: base_rate * (1 + skill_bonus) * decryptor_modifier
        """
        inventory = entity.components.get(Inventory)
        skills = entity.components.get(SkillTraining)
        
        t1_bpc = inventory.items[t1_bpc_id]
        item_category = t1_bpc.get("category", "module")
        
        # Base success rate
        base_rate = self.BASE_SUCCESS_RATES.get(item_category, 0.30)
        
        # Skill bonus (1% per level of each required science skill)
        skill_bonus = 0.0
        required_skills = t1_bpc.get("invention_skills", [])
        for skill_id in required_skills:
            skill_level = skills.trained_skills.get(skill_id, 0)
            skill_bonus += skill_level * self.SKILL_MULTIPLIER_PER_LEVEL
        
        # Decryptor modifier
        decryptor_modifier = 1.0
        if decryptor_id and decryptor_id in inventory.items:
            decryptor = inventory.items[decryptor_id]
            decryptor_modifier = 1.0 + decryptor.get("probability_modifier", 0.0)
        
        # Calculate final chance
        success_chance = base_rate * (1.0 + skill_bonus) * decryptor_modifier
        
        # Cap at 100%
        return min(success_chance, 1.0)
    
    def attempt_invention(self, entity: 'Entity', t1_bpc_id: str,
                         datacore1_id: str, datacore2_id: str,
                         decryptor_id: Optional[str] = None) -> Tuple[bool, Optional[Dict], str]:
        """
        Attempt to invent a T2 BPC
        
        Returns:
            Tuple[bool, Optional[Dict], str]: (success, t2_bpc_data, message)
        """
        # Validate invention is possible
        can_invent, error_msg = self.can_invent(entity, t1_bpc_id, 
                                                datacore1_id, datacore2_id,
                                                decryptor_id)
        if not can_invent:
            return False, None, error_msg
        
        # Get components
        inventory = entity.components.get(Inventory)
        t1_bpc = inventory.items[t1_bpc_id]
        
        # Calculate success chance
        success_chance = self.calculate_success_chance(entity, t1_bpc_id, decryptor_id)
        
        # Roll for success
        roll = random.random()
        success = roll < success_chance
        
        # Consume materials (always consumed, win or lose)
        del inventory.items[t1_bpc_id]
        del inventory.items[datacore1_id]
        del inventory.items[datacore2_id]
        if decryptor_id:
            decryptor = inventory.items.pop(decryptor_id, None)
        else:
            decryptor = None
        
        if not success:
            return False, None, f"Invention failed (chance was {success_chance:.1%})"
        
        # Create T2 BPC
        item_category = t1_bpc.get("category", "module")
        base_runs = self.DEFAULT_T2_RUNS.get(item_category, 1)
        
        # Apply decryptor modifiers
        runs_modifier = decryptor.get("runs_modifier", 0) if decryptor else 0
        me_modifier = decryptor.get("me_modifier", 0) if decryptor else 0
        te_modifier = decryptor.get("te_modifier", 0) if decryptor else 0
        
        t2_bpc = {
            "type": "blueprint",
            "name": t1_bpc["name"].replace("I", "II"),
            "item_id": t1_bpc["item_id"].replace("_t1", "_t2"),
            "tech_level": 2,
            "is_original": False,
            "runs": base_runs + runs_modifier,
            "me": 2 + me_modifier,  # T2 BPCs start at ME 2
            "te": 4 + te_modifier,  # T2 BPCs start at TE 4
            "category": item_category,
            "invention_skills": t1_bpc.get("invention_skills", []),
        }
        
        # Add to inventory
        t2_bpc_id = f"{t2_bpc['item_id']}_bpc_{id(t2_bpc)}"
        inventory.items[t2_bpc_id] = t2_bpc
        
        message = (f"Invention successful! Created {t2_bpc['name']} BPC with "
                  f"{t2_bpc['runs']} runs (chance was {success_chance:.1%})")
        
        return True, t2_bpc, message
    
    def start_research_agent(self, entity: 'Entity', agent_id: str, 
                            science_skill: str) -> Tuple[bool, str]:
        """
        Start a research project with an R&D agent for passive datacore generation
        
        Returns:
            Tuple[bool, str]: (success, message)
        """
        skills = entity.components.get(SkillTraining)
        if not skills:
            return False, "No skill training component"
        
        # Check if player has required skill
        if science_skill not in skills.trained_skills:
            return False, f"You must have {science_skill} trained to start research"
        
        skill_level = skills.trained_skills[science_skill]
        if skill_level < 1:
            return False, f"{science_skill} must be at least level 1"
        
        # Create or update research project
        if not hasattr(skills, "research_agents"):
            skills.research_agents = {}
        
        skills.research_agents[agent_id] = {
            "science_skill": science_skill,
            "skill_level": skill_level,
            "points_per_day": skill_level * 50,  # 50 RP per day per skill level
            "started_at": 0,  # Game time when started
        }
        
        return True, f"Started research with {agent_id} on {science_skill}"
    
    def collect_datacores(self, entity: 'Entity', agent_id: str,
                         days_elapsed: int) -> Tuple[bool, int, str]:
        """
        Collect datacores from R&D agent
        
        Returns:
            Tuple[bool, int, str]: (success, datacores_collected, message)
        """
        skills = entity.components.get(SkillTraining)
        if not skills or not hasattr(skills, "research_agents"):
            return False, 0, "No active research agents"
        
        if agent_id not in skills.research_agents:
            return False, 0, f"No research project with {agent_id}"
        
        agent_data = skills.research_agents[agent_id]
        science_skill = agent_data["science_skill"]
        points_per_day = agent_data["points_per_day"]
        
        # Calculate datacores earned (100 RP = 1 datacore)
        total_points = points_per_day * days_elapsed
        datacores = total_points // 100
        
        if datacores == 0:
            return False, 0, "Not enough research points accumulated yet (need 100 RP for 1 datacore)"
        
        # Add datacores to inventory
        inventory = entity.components.get(Inventory)
        if not inventory:
            return False, 0, "No inventory component"
        
        # Create datacore item
        datacore_type = science_skill.lower().replace(" ", "_")
        datacore_id = f"datacore_{datacore_type}_{id(agent_data)}"
        
        inventory.items[datacore_id] = {
            "type": "datacore",
            "name": f"{science_skill} Datacore",
            "quantity": datacores,
            "science_skill": science_skill,
        }
        
        message = f"Collected {datacores} {science_skill} Datacores from {agent_id}"
        return True, datacores, message
    
    def update(self, dt: float):
        """Update research agents (passive datacore generation)"""
        # In a full implementation, this would tick research progress
        # For now, datacores are collected on-demand via collect_datacores()
        pass
