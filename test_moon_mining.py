"""
Tests for Moon Mining System
Tests moon extraction, chunk fracturing, belt mining, and operation management
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World, Entity
from engine.components.game_components import (
    MoonMiningOperation, MoonResource, Structure
)
from engine.systems.moon_mining_system import (
    MoonMiningSystem, MoonManager, MOON_ORE_DATA, MOON_COMPOSITIONS
)


class TestMoonManager(unittest.TestCase):
    """Test moon resource management"""
    
    def setUp(self):
        """Set up moon manager"""
        self.manager = MoonManager()
    
    def test_register_moon(self):
        """Test registering a moon with custom composition"""
        composition = {"ubiquitous_moon_ore": 0.6, "common_moon_ore": 0.4}
        moon = self.manager.register_moon(
            moon_id="moon_1",
            moon_name="Test Moon I",
            system_id="sys_1",
            planet_id="planet_1",
            security=0.5,
            composition=composition
        )
        
        self.assertIsNotNone(moon)
        self.assertEqual(moon["moon_id"], "moon_1")
        self.assertEqual(moon["composition"], composition)
    
    def test_register_moon_auto_composition_highsec(self):
        """Test auto-generating highsec moon composition"""
        moon = self.manager.register_moon(
            moon_id="moon_hs",
            moon_name="Highsec Moon",
            system_id="sys_1",
            planet_id="planet_1",
            security=0.7
        )
        
        self.assertIn("ubiquitous_moon_ore", moon["composition"])
        total = sum(moon["composition"].values())
        self.assertAlmostEqual(total, 1.0)
    
    def test_register_moon_auto_composition_lowsec(self):
        """Test auto-generating lowsec moon composition"""
        moon = self.manager.register_moon(
            moon_id="moon_ls",
            moon_name="Lowsec Moon",
            system_id="sys_2",
            planet_id="planet_2",
            security=0.3
        )
        
        self.assertIn("rare_moon_ore", moon["composition"])
    
    def test_register_moon_auto_composition_nullsec(self):
        """Test auto-generating nullsec moon composition"""
        moon = self.manager.register_moon(
            moon_id="moon_ns",
            moon_name="Nullsec Moon",
            system_id="sys_3",
            planet_id="planet_3",
            security=-0.5
        )
        
        self.assertIn("legendary_moon_ore", moon["composition"])
    
    def test_get_moon(self):
        """Test retrieving a registered moon"""
        self.manager.register_moon(
            moon_id="moon_1",
            moon_name="Test Moon",
            system_id="sys_1",
            planet_id="planet_1",
            security=0.5
        )
        
        moon = self.manager.get_moon("moon_1")
        self.assertIsNotNone(moon)
        self.assertEqual(moon["moon_name"], "Test Moon")
    
    def test_get_moon_not_found(self):
        """Test getting a non-existent moon"""
        moon = self.manager.get_moon("nonexistent")
        self.assertIsNone(moon)
    
    def test_get_system_moons(self):
        """Test getting all moons in a system"""
        self.manager.register_moon("moon_1", "Moon 1", "sys_1", "p1", 0.5)
        self.manager.register_moon("moon_2", "Moon 2", "sys_1", "p2", 0.5)
        self.manager.register_moon("moon_3", "Moon 3", "sys_2", "p3", 0.3)
        
        sys1_moons = self.manager.get_system_moons("sys_1")
        self.assertEqual(len(sys1_moons), 2)
        
        sys2_moons = self.manager.get_system_moons("sys_2")
        self.assertEqual(len(sys2_moons), 1)


class TestMoonMiningSystem(unittest.TestCase):
    """Test moon mining operations"""
    
    def setUp(self):
        """Set up test world with moon mining infrastructure"""
        self.world = World()
        self.moon_manager = MoonManager()
        self.system = MoonMiningSystem(self.world, self.moon_manager)
        
        # Register a test moon
        self.moon_manager.register_moon(
            moon_id="test_moon",
            moon_name="Test Moon VII - Moon 3",
            system_id="test_system",
            planet_id="test_planet",
            security=0.5,
            composition={
                "ubiquitous_moon_ore": 0.50,
                "common_moon_ore": 0.35,
                "uncommon_moon_ore": 0.15,
            },
            yield_multiplier=1.0
        )
        
        # Create refinery entity
        self.refinery = self.world.create_entity()
        self.refinery.add_component(MoonMiningOperation())
        self.refinery.add_component(Structure(
            structure_type="athanor",
            structure_name="Test Refinery",
            state="online"
        ))
    
    def test_start_extraction(self):
        """Test starting a moon extraction"""
        result = self.system.start_extraction(
            self.refinery, "test_moon"
        )
        
        self.assertTrue(result)
        operation = self.refinery.get_component(MoonMiningOperation)
        self.assertEqual(operation.state, "extracting")
        self.assertEqual(operation.moon_id, "test_moon")
    
    def test_start_extraction_custom_duration(self):
        """Test starting extraction with custom duration"""
        result = self.system.start_extraction(
            self.refinery, "test_moon", duration=43200.0  # 12 hours
        )
        
        self.assertTrue(result)
        operation = self.refinery.get_component(MoonMiningOperation)
        self.assertEqual(operation.extraction_duration, 43200.0)
    
    def test_start_extraction_too_short(self):
        """Test extraction fails with duration less than 1 hour"""
        result = self.system.start_extraction(
            self.refinery, "test_moon", duration=1800.0  # 30 minutes
        )
        
        self.assertFalse(result)
    
    def test_start_extraction_unknown_moon(self):
        """Test extraction fails for unknown moon"""
        result = self.system.start_extraction(
            self.refinery, "nonexistent_moon"
        )
        
        self.assertFalse(result)
    
    def test_start_extraction_while_extracting(self):
        """Test can't start new extraction while one is active"""
        self.system.start_extraction(self.refinery, "test_moon")
        
        result = self.system.start_extraction(self.refinery, "test_moon")
        self.assertFalse(result)
    
    def test_cancel_extraction(self):
        """Test cancelling an extraction"""
        self.system.start_extraction(self.refinery, "test_moon")
        
        result = self.system.cancel_extraction(self.refinery)
        self.assertTrue(result)
        
        operation = self.refinery.get_component(MoonMiningOperation)
        self.assertEqual(operation.state, "idle")
    
    def test_cancel_extraction_not_extracting(self):
        """Test can't cancel when not extracting"""
        result = self.system.cancel_extraction(self.refinery)
        self.assertFalse(result)
    
    def test_extraction_progress_via_update(self):
        """Test extraction progress through update cycles"""
        self.system.start_extraction(
            self.refinery, "test_moon", duration=10000.0
        )
        
        # Simulate 5000 seconds (50% progress)
        self.system.update(5000.0)
        
        operation = self.refinery.get_component(MoonMiningOperation)
        self.assertAlmostEqual(operation.extraction_progress, 0.5, places=1)
        self.assertEqual(operation.state, "extracting")
    
    def test_extraction_completes_via_update(self):
        """Test extraction auto-completes when time expires"""
        self.system.start_extraction(
            self.refinery, "test_moon", duration=10000.0
        )
        
        # Simulate full extraction time
        self.system.update(10000.0)
        
        operation = self.refinery.get_component(MoonMiningOperation)
        self.assertEqual(operation.state, "ready")
        self.assertAlmostEqual(operation.extraction_progress, 1.0)
    
    def test_fracture_chunk(self):
        """Test fracturing the extracted moon chunk"""
        self.system.start_extraction(self.refinery, "test_moon")
        
        # Complete extraction
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        
        belt = self.system.fracture_chunk(self.refinery)
        
        self.assertIsNotNone(belt)
        self.assertIn("ubiquitous_moon_ore", belt)
        self.assertIn("common_moon_ore", belt)
        self.assertEqual(operation.state, "belt_active")
        self.assertEqual(operation.total_extractions, 1)
    
    def test_fracture_chunk_not_ready(self):
        """Test fracturing fails when not ready"""
        result = self.system.fracture_chunk(self.refinery)
        self.assertIsNone(result)
    
    def test_fracture_chunk_ore_volumes(self):
        """Test that fractured belt has correct ore proportions"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        
        belt = self.system.fracture_chunk(self.refinery)
        
        # 50% ubiquitous, 35% common, 15% uncommon of 20000 base
        self.assertAlmostEqual(belt["ubiquitous_moon_ore"], 10000.0, places=0)
        self.assertAlmostEqual(belt["common_moon_ore"], 7000.0, places=0)
        self.assertAlmostEqual(belt["uncommon_moon_ore"], 3000.0, places=0)
    
    def test_mine_from_belt(self):
        """Test mining ore from an active belt"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        mined = self.system.mine_from_belt(
            self.refinery, "ubiquitous_moon_ore", 500.0
        )
        
        self.assertIsNotNone(mined)
        self.assertAlmostEqual(mined, 500.0)
        
        # Belt should have less ore
        self.assertAlmostEqual(
            operation.belt_ores["ubiquitous_moon_ore"], 9500.0, places=0
        )
    
    def test_mine_from_belt_exceeds_available(self):
        """Test mining more than available caps at available amount"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        # Try to mine more than available
        available = operation.belt_ores["uncommon_moon_ore"]
        mined = self.system.mine_from_belt(
            self.refinery, "uncommon_moon_ore", available + 5000.0
        )
        
        self.assertAlmostEqual(mined, available)
        self.assertNotIn("uncommon_moon_ore", operation.belt_ores)
    
    def test_mine_from_belt_unknown_ore(self):
        """Test mining unknown ore type from belt fails"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        mined = self.system.mine_from_belt(
            self.refinery, "fake_ore", 100.0
        )
        
        self.assertIsNone(mined)
    
    def test_mine_from_belt_no_active_belt(self):
        """Test mining fails when no belt is active"""
        mined = self.system.mine_from_belt(
            self.refinery, "ubiquitous_moon_ore", 100.0
        )
        
        self.assertIsNone(mined)
    
    def test_belt_depleted_resets_state(self):
        """Test that depleting all ore in belt resets state to idle"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        # Mine all ore types
        for ore_type in list(operation.belt_ores.keys()):
            amount = operation.belt_ores[ore_type]
            self.system.mine_from_belt(self.refinery, ore_type, amount)
        
        self.assertEqual(operation.state, "idle")
    
    def test_belt_expires_via_update(self):
        """Test that belt auto-expires after lifetime"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        # Simulate 48+ hours
        self.system.update(172800.0)
        
        self.assertEqual(operation.state, "idle")
        self.assertEqual(len(operation.belt_ores), 0)
    
    def test_get_operation_status_idle(self):
        """Test getting status when idle"""
        status = self.system.get_operation_status(self.refinery)
        
        self.assertIsNotNone(status)
        self.assertEqual(status["state"], "idle")
    
    def test_get_operation_status_extracting(self):
        """Test getting status while extracting"""
        self.system.start_extraction(self.refinery, "test_moon")
        self.system.update(5000.0)
        
        status = self.system.get_operation_status(self.refinery)
        
        self.assertEqual(status["state"], "extracting")
        self.assertIn("progress", status)
        self.assertIn("time_remaining", status)
    
    def test_get_operation_status_belt_active(self):
        """Test getting status with active belt"""
        self.system.start_extraction(self.refinery, "test_moon")
        operation = self.refinery.get_component(MoonMiningOperation)
        operation.state = "ready"
        self.system.fracture_chunk(self.refinery)
        
        status = self.system.get_operation_status(self.refinery)
        
        self.assertEqual(status["state"], "belt_active")
        self.assertIn("belt_ores", status)
        self.assertIn("total_belt_volume", status)
    
    def test_get_moon_ore_info(self):
        """Test getting moon ore information"""
        info = self.system.get_moon_ore_info("ubiquitous_moon_ore")
        
        self.assertIsNotNone(info)
        self.assertEqual(info["rarity"], "R4")
        self.assertIn("minerals", info)
    
    def test_get_moon_ore_info_unknown(self):
        """Test getting info for unknown ore type"""
        info = self.system.get_moon_ore_info("fake_ore")
        self.assertIsNone(info)
    
    def test_get_all_moon_ores(self):
        """Test getting list of all moon ore types"""
        ores = self.system.get_all_moon_ores()
        
        self.assertEqual(len(ores), 10)
        self.assertIn("ubiquitous_moon_ore", ores)
        self.assertIn("legendary_moon_ore", ores)
    
    def test_yield_multiplier(self):
        """Test that yield multiplier affects ore generation"""
        # Register a high-yield moon
        self.moon_manager.register_moon(
            moon_id="rich_moon",
            moon_name="Rich Moon",
            system_id="test_system",
            planet_id="test_planet",
            security=0.5,
            composition={"ubiquitous_moon_ore": 1.0},
            yield_multiplier=2.0
        )
        
        # Create second refinery
        refinery2 = self.world.create_entity()
        refinery2.add_component(MoonMiningOperation())
        
        self.system.start_extraction(refinery2, "rich_moon")
        operation = refinery2.get_component(MoonMiningOperation)
        operation.state = "ready"
        
        belt = self.system.fracture_chunk(refinery2)
        
        # 2x yield: 20000 * 1.0 * 2.0 = 40000 m3
        self.assertAlmostEqual(belt["ubiquitous_moon_ore"], 40000.0, places=0)
    
    def test_multiple_extraction_cycles(self):
        """Test running multiple extraction cycles"""
        operation = self.refinery.get_component(MoonMiningOperation)
        
        for i in range(3):
            self.system.start_extraction(self.refinery, "test_moon")
            operation.state = "ready"
            self.system.fracture_chunk(self.refinery)
            
            # Deplete belt
            for ore_type in list(operation.belt_ores.keys()):
                self.system.mine_from_belt(
                    self.refinery, ore_type, operation.belt_ores[ore_type]
                )
        
        self.assertEqual(operation.total_extractions, 3)
        self.assertGreater(operation.total_ore_generated, 0)
    
    def test_moon_compositions_sum_to_one(self):
        """Test that all predefined compositions sum to 1.0"""
        for sec_name, composition in MOON_COMPOSITIONS.items():
            total = sum(composition.values())
            self.assertAlmostEqual(
                total, 1.0, places=2,
                msg=f"{sec_name} composition sums to {total}, expected 1.0"
            )
    
    def test_moon_ore_data_complete(self):
        """Test that all moon ore types have required fields"""
        for ore_type, data in MOON_ORE_DATA.items():
            self.assertIn("rarity", data, f"{ore_type} missing rarity")
            self.assertIn("name", data, f"{ore_type} missing name")
            self.assertIn("minerals", data, f"{ore_type} missing minerals")
            self.assertIn("volume_per_unit", data, f"{ore_type} missing volume_per_unit")


if __name__ == '__main__':
    unittest.main()
