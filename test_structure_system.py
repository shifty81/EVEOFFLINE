#!/usr/bin/env python3
"""
Tests for the Structure System
Tests structure deployment, fuel management, services, and lifecycle.
"""

import unittest
import time
from engine.core.ecs import World
from engine.systems.structure_system import StructureSystem
from engine.components.game_components import (
    Structure, StructureService, Position, Player, 
    CorporationMembership, Inventory
)


class TestStructureSystem(unittest.TestCase):
    """Test cases for structure system"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.world = World()
        self.structure_system = StructureSystem(self.world)
        
        # Create a test player
        player_entity = self.world.create_entity()
        self.player_id = player_entity.id
        player_comp = Player(
            character_name="TestPilot",
            isk=1000000000.0
        )
        player_entity.add_component(player_comp)
        
        # Add corporation membership
        corp_comp = CorporationMembership(
            corporation_id="test_corp",
            corporation_name="Test Corporation",
            role="ceo"
        )
        player_entity.add_component(corp_comp)
    
    def test_deploy_astrahus(self):
        """Test deploying an Astrahus citadel"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Test Citadel",
            "jita",
            100.0, 200.0, 300.0
        )
        
        self.assertIsNotNone(structure_id)
        
        # Check structure component
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertIsNotNone(structure_comp)
        self.assertEqual(structure_comp.structure_type, "astrahus")
        self.assertEqual(structure_comp.structure_name, "Test Citadel")
        self.assertEqual(structure_comp.state, "anchoring")
        self.assertEqual(structure_comp.owner_corporation_id, "test_corp")
        self.assertEqual(structure_comp.owner_player_id, self.player_id)
        
        # Check position
        pos_comp = self.world.get_entity(structure_id).get_component(Position)
        self.assertIsNotNone(pos_comp)
        self.assertEqual(pos_comp.x, 100.0)
        self.assertEqual(pos_comp.y, 200.0)
        self.assertEqual(pos_comp.z, 300.0)
    
    def test_deploy_orbital_skyhook(self):
        """Test deploying an Orbital Skyhook"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "orbital_skyhook",
            "Mining Skyhook",
            "nullsec_system",
            50.0, 100.0, 150.0
        )
        
        self.assertIsNotNone(structure_id)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.structure_type, "orbital_skyhook")
        self.assertEqual(structure_comp.structure_name, "Mining Skyhook")
    
    def test_deploy_sovereignty_hub(self):
        """Test deploying a Sovereignty Hub"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "sovereignty_hub",
            "Alliance Hub",
            "nullsec_system",
            0.0, 0.0, 0.0
        )
        
        self.assertIsNotNone(structure_id)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.structure_type, "sovereignty_hub")
        self.assertEqual(structure_comp.anchoring_time_remaining, 172800.0)  # 48 hours
    
    def test_deploy_metenox_moon_drill(self):
        """Test deploying a Metenox Moon Drill"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "metenox_moon_drill",
            "Moon Drill Alpha",
            "nullsec_system",
            1000.0, 2000.0, 3000.0
        )
        
        self.assertIsNotNone(structure_id)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.structure_type, "metenox_moon_drill")
        self.assertEqual(structure_comp.anchoring_time_remaining, 43200.0)  # 12 hours
    
    def test_structure_anchoring(self):
        """Test structure anchoring process"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Fast Anchor Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.state, "anchoring")
        
        # Simulate time passing
        initial_time = structure_comp.anchoring_time_remaining
        self.structure_system.update(3600.0)  # 1 hour
        
        self.assertLess(structure_comp.anchoring_time_remaining, initial_time)
        self.assertEqual(structure_comp.state, "anchoring")
        
        # Complete anchoring
        self.structure_system.update(structure_comp.anchoring_time_remaining + 1.0)
        self.assertEqual(structure_comp.state, "online")
        self.assertEqual(structure_comp.anchoring_time_remaining, 0.0)
    
    def test_add_fuel_to_structure(self):
        """Test adding fuel to a structure"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Fueling Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Add fuel
        success = self.structure_system.add_fuel(structure_id, "fuel_blocks", 1000)
        self.assertTrue(success)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.fuel_bay["fuel_blocks"], 1000)
        
        # Add more fuel
        success = self.structure_system.add_fuel(structure_id, "fuel_blocks", 500)
        self.assertTrue(success)
        self.assertEqual(structure_comp.fuel_bay["fuel_blocks"], 1500)
    
    def test_fuel_capacity_limit(self):
        """Test that fuel bay has capacity limit"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Capacity Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        capacity = structure_comp.fuel_bay_capacity
        
        # Try to overfill
        success = self.structure_system.add_fuel(structure_id, "fuel_blocks", int(capacity) + 1000)
        self.assertFalse(success)
    
    def test_fuel_consumption(self):
        """Test fuel consumption over time"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Consumption Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Complete anchoring
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.structure_system.update(structure_comp.anchoring_time_remaining + 1.0)
        
        # Add fuel
        self.structure_system.add_fuel(structure_id, "fuel_blocks", 1000)
        
        # Set initial fuel check time
        structure_comp.last_fuel_check = time.time()
        
        # Wait and update (simulate 1 hour passing)
        time.sleep(0.1)  # Small delay for testing
        structure_comp.last_fuel_check = time.time() - 3600.0  # Fake 1 hour ago
        
        initial_fuel = structure_comp.fuel_bay["fuel_blocks"]
        self.structure_system.update(0.1)
        
        # Fuel should be consumed
        self.assertLess(structure_comp.fuel_bay["fuel_blocks"], initial_fuel)
    
    def test_activate_service(self):
        """Test activating a service on a structure"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Service Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Complete anchoring
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.structure_system.update(structure_comp.anchoring_time_remaining + 1.0)
        
        # Activate market service
        success = self.structure_system.activate_service(structure_id, "market")
        self.assertTrue(success)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertTrue(structure_comp.services.get("market", False))
        
        # Activate manufacturing service
        success = self.structure_system.activate_service(structure_id, "manufacturing")
        self.assertTrue(success)
        self.assertTrue(structure_comp.services.get("manufacturing", False))
    
    def test_cannot_activate_service_while_anchoring(self):
        """Test that services cannot be activated while structure is anchoring"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Anchoring Service Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Try to activate service while anchoring
        success = self.structure_system.activate_service(structure_id, "market")
        self.assertFalse(success)
    
    def test_get_structures_in_system(self):
        """Test getting all structures in a system"""
        # Deploy multiple structures
        structure1 = self.structure_system.deploy_structure(self.player_id, "astrahus", "Structure 1", "jita", 0.0, 0.0, 0.0
        )
        structure2 = self.structure_system.deploy_structure(self.player_id, "orbital_skyhook", "Structure 2", "jita", 100.0, 0.0, 0.0
        )
        structure3 = self.structure_system.deploy_structure(self.player_id, "astrahus", "Structure 3", "amarr", 0.0, 0.0, 0.0
        )
        
        jita_structures = self.structure_system.get_structures_in_system("jita")
        self.assertEqual(len(jita_structures), 2)
        self.assertIn(structure1, jita_structures)
        self.assertIn(structure2, jita_structures)
        
        amarr_structures = self.structure_system.get_structures_in_system("amarr")
        self.assertEqual(len(amarr_structures), 1)
        self.assertIn(structure3, amarr_structures)
    
    def test_get_structure_info(self):
        """Test getting structure information"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Info Test",
            "jita",
            123.0, 456.0, 789.0
        )
        
        info = self.structure_system.get_structure_info(structure_id)
        
        self.assertIsNotNone(info)
        self.assertEqual(info["name"], "Info Test")
        self.assertEqual(info["type"], "astrahus")
        self.assertEqual(info["state"], "anchoring")
        self.assertEqual(info["owner_corp"], "test_corp")
        self.assertEqual(info["owner_player"], self.player_id)
        self.assertEqual(info["position"], (123.0, 456.0, 789.0))
        self.assertGreater(info["hull_max"], 0)
        self.assertGreater(info["shield_max"], 0)
        self.assertGreater(info["armor_max"], 0)
    
    def test_unanchor_structure(self):
        """Test unanchoring a structure"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Unanchor Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Complete anchoring
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.structure_system.update(structure_comp.anchoring_time_remaining + 1.0)
        
        # Unanchor
        success = self.structure_system.unanchor_structure(structure_id, self.player_id)
        self.assertTrue(success)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.state, "unanchoring")
        self.assertGreater(structure_comp.anchoring_time_remaining, 0)
    
    def test_cannot_unanchor_while_anchoring(self):
        """Test that structures cannot be unanchored while still anchoring"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "No Unanchor Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Try to unanchor while still anchoring
        success = self.structure_system.unanchor_structure(structure_id, self.player_id)
        self.assertFalse(success)
    
    def test_structure_health_pools(self):
        """Test that structures have proper health pools"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Health Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        
        # Check hull
        self.assertGreater(structure_comp.hull_hp, 0)
        self.assertEqual(structure_comp.hull_hp, structure_comp.hull_max)
        
        # Check shield
        self.assertGreater(structure_comp.shield_hp, 0)
        self.assertEqual(structure_comp.shield_hp, structure_comp.shield_max)
        
        # Check armor
        self.assertGreater(structure_comp.armor_hp, 0)
        self.assertEqual(structure_comp.armor_hp, structure_comp.armor_max)
    
    def test_structure_fitting_slots(self):
        """Test that structures have fitting slots"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Fitting Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        
        # Check that structure has CPU and PowerGrid
        self.assertGreater(structure_comp.cpu_max, 0)
        self.assertGreater(structure_comp.powergrid_max, 0)
        self.assertEqual(structure_comp.cpu, 0.0)
        self.assertEqual(structure_comp.powergrid, 0.0)
    
    def test_destroy_structure(self):
        """Test destroying a structure"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Destroy Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Verify structure exists
        entity = self.world.get_entity(structure_id)
        self.assertIsNotNone(entity)
        structure_comp = entity.get_component(Structure)
        self.assertIsNotNone(structure_comp)
        
        # Destroy structure
        self.structure_system.destroy_structure(structure_id)
        
        # Verify structure is gone
        entity = self.world.get_entity(structure_id)
        self.assertIsNone(entity)
        
        # Verify removed from system tracking
        jita_structures = self.structure_system.get_structures_in_system("jita")
        self.assertNotIn(structure_id, jita_structures)
    
    def test_multiple_fuel_types(self):
        """Test structure can hold multiple fuel types"""
        structure_id = self.structure_system.deploy_structure(self.player_id,
            "astrahus",
            "Multi-Fuel Test",
            "jita",
            0.0, 0.0, 0.0
        )
        
        # Add different fuel types
        self.structure_system.add_fuel(structure_id, "helium_isotopes", 500)
        self.structure_system.add_fuel(structure_id, "nitrogen_isotopes", 300)
        
        structure_comp = self.world.get_entity(structure_id).get_component(Structure)
        self.assertEqual(structure_comp.fuel_bay["helium_isotopes"], 500)
        self.assertEqual(structure_comp.fuel_bay["nitrogen_isotopes"], 300)
        self.assertEqual(len(structure_comp.fuel_bay), 2)


def run_tests():
    """Run all tests and print results"""
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestStructureSystem)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
