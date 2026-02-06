"""
Tests for Ice Mining System
Tests ice harvesting, ice fields, reprocessing, and integration with existing systems
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.core.ecs import World
from engine.components.game_components import (
    Position, IceHarvester, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Fitting
)
from engine.systems.ice_mining_system import IceMiningSystem, IceFieldManager, IceField


class TestIceField(unittest.TestCase):
    """Test IceField class"""
    
    def test_create_ice_field(self):
        """Test creating an ice field"""
        field = IceField("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        
        self.assertEqual(field.id, "field_1")
        self.assertEqual(field.ice_type, "clear_icicle")
        self.assertEqual(field.system_id, "jita")
        self.assertEqual(field.remaining_units, 1000)
        self.assertFalse(field.is_depleted())
    
    def test_harvest_ice(self):
        """Test harvesting ice from field"""
        field = IceField("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        
        harvested = field.harvest(1)
        self.assertEqual(harvested, 1)
        self.assertEqual(field.remaining_units, 999)
    
    def test_deplete_ice_field(self):
        """Test depleting an ice field"""
        field = IceField("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        field.remaining_units = 1
        
        harvested = field.harvest(1)
        self.assertEqual(harvested, 1)
        self.assertTrue(field.is_depleted())
    
    def test_cannot_overharvest(self):
        """Test that you cannot harvest more than available"""
        field = IceField("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        field.remaining_units = 5
        
        harvested = field.harvest(10)
        self.assertEqual(harvested, 5)
        self.assertTrue(field.is_depleted())


class TestIceFieldManager(unittest.TestCase):
    """Test IceFieldManager class"""
    
    def setUp(self):
        self.manager = IceFieldManager()
    
    def test_create_and_get_field(self):
        """Test creating and retrieving ice fields"""
        field = self.manager.create_ice_field(
            "field_1", "gelidus", "system_1", (100.0, 200.0, 300.0)
        )
        
        self.assertEqual(field.ice_type, "gelidus")
        
        retrieved = self.manager.get_ice_field("field_1")
        self.assertEqual(retrieved.id, "field_1")
        self.assertEqual(retrieved.ice_type, "gelidus")
    
    def test_get_fields_in_system(self):
        """Test getting all fields in a system"""
        self.manager.create_ice_field("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        self.manager.create_ice_field("field_2", "blue_ice", "jita", (100.0, 0.0, 0.0))
        self.manager.create_ice_field("field_3", "gelidus", "amarr", (0.0, 0.0, 0.0))
        
        jita_fields = self.manager.get_fields_in_system("jita")
        self.assertEqual(len(jita_fields), 2)
        
        amarr_fields = self.manager.get_fields_in_system("amarr")
        self.assertEqual(len(amarr_fields), 1)
    
    def test_remove_field(self):
        """Test removing an ice field"""
        self.manager.create_ice_field("field_1", "clear_icicle", "jita", (0.0, 0.0, 0.0))
        
        self.assertIsNotNone(self.manager.get_ice_field("field_1"))
        
        self.manager.remove_field("field_1")
        
        self.assertIsNone(self.manager.get_ice_field("field_1"))


class TestIceMiningSystem(unittest.TestCase):
    """Test IceMiningSystem"""
    
    def setUp(self):
        self.world = World()
        self.ice_field_manager = IceFieldManager()
        self.system = IceMiningSystem(self.world, self.ice_field_manager)
        
        # Create test ice field
        self.field = self.ice_field_manager.create_ice_field(
            "test_field", "clear_icicle", "jita", (500.0, 0.0, 0.0)
        )
        
        # Create test entity
        self.entity = self.world.create_entity()
        self.entity.add_component(Position(x=0.0, y=0.0, z=0.0))
        self.entity.add_component(IceHarvester(
            harvester_type="Ice Harvester I",
            cycle_time=300.0,
            optimal_range=12000.0,
            capacitor_usage=540.0
        ))
        self.entity.add_component(Capacitor(capacitor=1000.0, capacitor_max=1000.0))
        self.entity.add_component(MiningYield())
        self.entity.add_component(OreHold(ore_hold_capacity=10000.0))
    
    def test_start_harvesting(self):
        """Test starting ice harvesting"""
        result = self.system.start_harvesting(self.entity, "test_field")
        
        self.assertTrue(result)
        ice_harvester = self.entity.get_component(IceHarvester)
        self.assertTrue(ice_harvester.is_active)
        self.assertEqual(ice_harvester.target_ice_id, "test_field")
    
    def test_cannot_start_out_of_range(self):
        """Test that harvesting fails when out of range"""
        # Move entity far away
        position = self.entity.get_component(Position)
        position.x = 100000.0
        
        result = self.system.start_harvesting(self.entity, "test_field")
        
        self.assertFalse(result)
    
    def test_cannot_start_without_capacitor(self):
        """Test that harvesting fails without enough capacitor"""
        capacitor = self.entity.get_component(Capacitor)
        capacitor.capacitor = 100.0  # Not enough
        
        result = self.system.start_harvesting(self.entity, "test_field")
        
        self.assertFalse(result)
    
    def test_stop_harvesting(self):
        """Test stopping ice harvesting"""
        self.system.start_harvesting(self.entity, "test_field")
        ice_harvester = self.entity.get_component(IceHarvester)
        self.assertTrue(ice_harvester.is_active)
        
        self.system.stop_harvesting(self.entity)
        
        self.assertFalse(ice_harvester.is_active)
        self.assertIsNone(ice_harvester.target_ice_id)
    
    def test_complete_harvesting_cycle(self):
        """Test completing a full harvesting cycle"""
        self.system.start_harvesting(self.entity, "test_field")
        
        # Simulate cycle completion
        ice_harvester = self.entity.get_component(IceHarvester)
        ice_harvester.current_cycle = ice_harvester.cycle_time
        
        # Process update
        self.system.update(0.1)
        
        # Check results
        ore_hold = self.entity.get_component(OreHold)
        self.assertIn("clear_icicle", ore_hold.ice)
        self.assertEqual(ore_hold.ice["clear_icicle"], 1)
        self.assertEqual(ore_hold.ore_hold_used, 1000.0)  # 1 unit = 1000 m³
        
        # Check field depletion
        self.assertEqual(self.field.remaining_units, 999)
        
        # Check capacitor consumption
        capacitor = self.entity.get_component(Capacitor)
        self.assertEqual(capacitor.capacitor, 460.0)  # 1000 - 540
    
    def test_harvesting_with_skills(self):
        """Test cycle time reduction with skills"""
        # Add Ice Harvesting skill at level 5
        skills = Skills()
        skills.skills = {'ice_harvesting': 5}
        self.entity.add_component(skills)
        
        # Calculate cycle time multiplier
        multiplier = self.system._calculate_cycle_time_multiplier(self.entity)
        
        # Should be 75% of base (25% reduction)
        self.assertAlmostEqual(multiplier, 0.75, places=2)
    
    def test_harvesting_with_upgrades(self):
        """Test cycle time reduction with modules"""
        fitting = Fitting()
        fitting.low_slots = {
            'low_1': {
                'id': 'ice_harvester_upgrade_i',
                'bonuses': {'ice_harvesting_cycle_time': -0.05}
            }
        }
        self.entity.add_component(fitting)
        
        multiplier = self.system._calculate_cycle_time_multiplier(self.entity)
        
        # Should be 95% of base (5% reduction)
        self.assertAlmostEqual(multiplier, 0.95, places=2)
    
    def test_storage_in_ore_hold(self):
        """Test ice is stored in ore hold"""
        self.system.start_harvesting(self.entity, "test_field")
        ice_harvester = self.entity.get_component(IceHarvester)
        ice_harvester.current_cycle = ice_harvester.cycle_time
        
        self.system.update(0.1)
        
        ore_hold = self.entity.get_component(OreHold)
        self.assertIn("clear_icicle", ore_hold.ice)
        self.assertEqual(ore_hold.ice["clear_icicle"], 1)
    
    def test_storage_overflow_to_cargo(self):
        """Test ice overflows to cargo when ore hold is full"""
        # Fill ore hold almost to capacity
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ore_hold_used = 9500.0  # Only 500 m³ left
        
        # Add regular cargo
        self.entity.add_component(Inventory(cargo_capacity=5000.0))
        
        self.system.start_harvesting(self.entity, "test_field")
        ice_harvester = self.entity.get_component(IceHarvester)
        ice_harvester.current_cycle = ice_harvester.cycle_time
        
        self.system.update(0.1)
        
        # Ice should be in cargo since ore hold was too full
        inventory = self.entity.get_component(Inventory)
        self.assertIn("clear_icicle", inventory.items)
    
    def test_multiple_harvest_cycles(self):
        """Test multiple harvesting cycles"""
        self.system.start_harvesting(self.entity, "test_field")
        
        # Complete 3 cycles
        for _ in range(3):
            ice_harvester = self.entity.get_component(IceHarvester)
            ice_harvester.current_cycle = ice_harvester.cycle_time
            self.system.update(0.1)
            
            # Recharge capacitor for next cycle
            capacitor = self.entity.get_component(Capacitor)
            capacitor.capacitor = capacitor.capacitor_max
        
        ore_hold = self.entity.get_component(OreHold)
        self.assertEqual(ore_hold.ice["clear_icicle"], 3)
        self.assertEqual(self.field.remaining_units, 997)
    
    def test_stop_when_field_depleted(self):
        """Test harvesting stops when field is depleted"""
        # Set field to almost depleted
        self.field.remaining_units = 1
        
        self.system.start_harvesting(self.entity, "test_field")
        ice_harvester = self.entity.get_component(IceHarvester)
        ice_harvester.current_cycle = ice_harvester.cycle_time
        
        # Complete one cycle (depletes field)
        self.system.update(0.1)
        
        # Try to complete another cycle
        ice_harvester.current_cycle = ice_harvester.cycle_time
        self.system.update(0.1)
        
        # Should have stopped
        self.assertFalse(ice_harvester.is_active)
        
        # Should only have harvested 1 unit
        ore_hold = self.entity.get_component(OreHold)
        self.assertEqual(ore_hold.ice.get("clear_icicle", 0), 1)
    
    def test_stop_when_cargo_full(self):
        """Test harvesting stops when cargo is full"""
        # Fill ore hold to capacity
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ore_hold_used = ore_hold.ore_hold_capacity
        
        self.system.start_harvesting(self.entity, "test_field")
        ice_harvester = self.entity.get_component(IceHarvester)
        ice_harvester.current_cycle = ice_harvester.cycle_time
        
        self.system.update(0.1)
        
        # Should have stopped due to no space
        self.assertFalse(ice_harvester.is_active)
    
    def test_get_nearby_ice_fields(self):
        """Test finding nearby ice fields"""
        # Create multiple fields
        self.ice_field_manager.create_ice_field("field_2", "blue_ice", "jita", (1000.0, 0.0, 0.0))
        self.ice_field_manager.create_ice_field("field_3", "gelidus", "jita", (50000.0, 0.0, 0.0))
        
        nearby = self.system.get_nearby_ice_fields(self.entity, max_distance=15000.0)
        
        # Should find test_field and field_2, but not field_3 (too far)
        self.assertEqual(len(nearby), 2)
        self.assertLess(nearby[0][1], nearby[1][1])  # Sorted by distance
    
    def test_get_harvesting_info(self):
        """Test getting harvesting information"""
        self.system.start_harvesting(self.entity, "test_field")
        
        info = self.system.get_harvesting_info(self.entity)
        
        self.assertTrue(info["is_active"])
        self.assertEqual(info["base_cycle_time"], 300.0)
        self.assertEqual(info["yield_per_cycle"], 1.0)
        self.assertEqual(info["optimal_range"], 12000.0)
        self.assertEqual(info["target_field_id"], "test_field")


class TestIceReprocessing(unittest.TestCase):
    """Test ice reprocessing mechanics"""
    
    def setUp(self):
        self.world = World()
        self.ice_field_manager = IceFieldManager()
        self.system = IceMiningSystem(self.world, self.ice_field_manager)
        
        self.entity = self.world.create_entity()
        self.entity.add_component(OreHold())
        self.entity.add_component(Skills())
    
    def test_reprocess_clear_icicle(self):
        """Test reprocessing Clear Icicle"""
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 10
        ore_hold.ore_hold_used = 10000.0
        
        products = self.system.reprocess_ice(self.entity, "clear_icicle", 10)
        
        self.assertIn("heavy_water", products)
        self.assertEqual(products["heavy_water"], 5000)  # 500 per unit × 10 units
    
    def test_reprocess_with_skills(self):
        """Test reprocessing with skills increases yield"""
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ice["gelidus"] = 1
        
        # Add reprocessing skills
        skills = self.entity.get_component(Skills)
        skills.skills = {
            'reprocessing': 5,
            'ice_processing': 5
        }
        
        products = self.system.reprocess_ice(self.entity, "gelidus", 1)
        
        # Base: 200 H, 300 He isotopes, 100 heavy water
        # With skills: +15% (reprocessing) +10% (ice processing) = +25%
        self.assertEqual(products["hydrogen"], 250)  # 200 * 1.25
        self.assertEqual(products["helium_isotopes"], 375)  # 300 * 1.25
        self.assertEqual(products["heavy_water"], 125)  # 100 * 1.25
    
    def test_reprocess_removes_ice(self):
        """Test reprocessing removes ice from storage"""
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 5
        ore_hold.ore_hold_used = 5000.0
        
        self.system.reprocess_ice(self.entity, "clear_icicle", 3)
        
        self.assertEqual(ore_hold.ice["clear_icicle"], 2)
        self.assertEqual(ore_hold.ore_hold_used, 2000.0)
    
    def test_cannot_reprocess_without_ice(self):
        """Test that reprocessing fails without enough ice"""
        ore_hold = self.entity.get_component(OreHold)
        ore_hold.ice["clear_icicle"] = 2
        
        products = self.system.reprocess_ice(self.entity, "clear_icicle", 5)
        
        self.assertEqual(products, {})  # Should return empty dict


if __name__ == '__main__':
    # Run tests
    unittest.main(verbosity=2)
