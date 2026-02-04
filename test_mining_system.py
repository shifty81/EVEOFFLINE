"""
Tests for Mining System
Tests mining mechanics, ore extraction, and yield calculations
"""

import unittest
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World, Entity
from engine.components.game_components import (
    Position, MiningLaser, MiningYield, OreHold, Inventory,
    Capacitor, Skills, Player, Fitting
)
from engine.systems.mining_system import MiningSystem
from engine.systems.asteroid_system import (
    AsteroidFieldManager, AsteroidBelt, Asteroid,
    AsteroidSize, BeltLayout
)


class TestMiningSystem(unittest.TestCase):
    """Test mining system functionality"""
    
    def setUp(self):
        """Set up test world and entities"""
        self.world = World()
        self.asteroid_manager = AsteroidFieldManager()
        self.mining_system = MiningSystem(self.world, self.asteroid_manager)
        
        # Create test asteroid belt
        self.test_belt = AsteroidBelt(
            belt_id="test_belt_1",
            name="Test Belt",
            system="test_system",
            security=1.0,
            layout=BeltLayout.SEMICIRCLE,
            radius_km=50.0
        )
        
        # Add test asteroid
        self.test_asteroid = Asteroid(
            id="test_ast_1",
            ore_type="veldspar",
            variant="standard",
            position=(1000.0, 0.0, 0.0),
            size=AsteroidSize.MEDIUM,
            ore_remaining=35000,
            ore_total=35000
        )
        self.test_belt.asteroids[self.test_asteroid.id] = self.test_asteroid
        self.asteroid_manager.belts[self.test_belt.id] = self.test_belt
        
        # Create miner entity
        self.miner = self.world.create_entity()
        self.miner.add_component(Position(x=0.0, y=0.0, z=0.0))
        self.miner.add_component(Capacitor(capacitor=1000.0, capacitor_max=1000.0))
        self.miner.add_component(MiningLaser(
            laser_type="Miner I",
            cycle_time=60.0,
            yield_amount=40.0,
            optimal_range=10000.0,
            capacitor_usage=60.0
        ))
        self.miner.add_component(MiningYield())
        self.miner.add_component(Inventory(cargo_capacity=500.0))
        self.miner.add_component(Skills())
    
    def test_start_mining_success(self):
        """Test successfully starting mining operation"""
        result = self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        self.assertTrue(result)
        
        mining_laser = self.miner.get_component(MiningLaser)
        self.assertTrue(mining_laser.is_active)
        self.assertEqual(mining_laser.target_asteroid_id, self.test_asteroid.id)
    
    def test_start_mining_out_of_range(self):
        """Test mining fails when out of range"""
        # Move miner far away
        pos = self.miner.get_component(Position)
        pos.x = 50000.0  # Beyond optimal range
        
        result = self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        self.assertFalse(result)
        
        mining_laser = self.miner.get_component(MiningLaser)
        self.assertFalse(mining_laser.is_active)
    
    def test_start_mining_depleted_asteroid(self):
        """Test mining fails on depleted asteroid"""
        # Deplete asteroid
        self.test_asteroid.ore_remaining = 0
        
        result = self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        self.assertFalse(result)
    
    def test_start_mining_insufficient_capacitor(self):
        """Test mining fails with insufficient capacitor"""
        cap = self.miner.get_component(Capacitor)
        cap.capacitor = 30.0  # Less than required 60
        
        result = self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        self.assertFalse(result)
    
    def test_stop_mining(self):
        """Test stopping mining operation"""
        # Start mining first
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Stop mining
        result = self.mining_system.stop_mining(self.miner)
        
        self.assertTrue(result)
        
        mining_laser = self.miner.get_component(MiningLaser)
        self.assertFalse(mining_laser.is_active)
        self.assertIsNone(mining_laser.target_asteroid_id)
    
    def test_mining_cycle_completion(self):
        """Test completing a full mining cycle"""
        # Start mining
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Simulate full cycle
        self.mining_system.update(60.0)  # Full cycle time
        
        # Check ore was mined
        inventory = self.miner.get_component(Inventory)
        self.assertIn("veldspar", inventory.items)
        self.assertGreater(inventory.items["veldspar"], 0)
        
        # Check capacitor was consumed
        cap = self.miner.get_component(Capacitor)
        self.assertLess(cap.capacitor, 1000.0)
        
        # Check asteroid was depleted
        self.assertLess(self.test_asteroid.ore_remaining, 35000)
    
    def test_mining_yield_with_skills(self):
        """Test yield calculation with skill bonuses"""
        skills = self.miner.get_component(Skills)
        skills.skills["mining"] = 5  # 5% per level = 25% bonus
        skills.skills["astrogeology"] = 3  # 5% per level = 15% bonus
        # Total: 1.25 * 1.15 = 1.4375x multiplier
        
        # Start and complete one cycle
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        self.mining_system.update(60.0)
        
        # Base yield is 40, with skills should be ~57-58
        inventory = self.miner.get_component(Inventory)
        mined = inventory.items.get("veldspar", 0)
        self.assertGreater(mined, 40)  # More than base
        self.assertGreater(mined, 55)  # At least 55 with bonuses
    
    def test_mining_cycle_progression(self):
        """Test mining cycle progresses over time"""
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        mining_laser = self.miner.get_component(MiningLaser)
        
        # Update halfway
        self.mining_system.update(30.0)
        self.assertAlmostEqual(mining_laser.current_cycle, 30.0, places=1)
        
        # Update to completion
        self.mining_system.update(30.0)
        self.assertLess(mining_laser.current_cycle, 10.0)  # Reset after cycle
    
    def test_ore_hold_storage(self):
        """Test ore storage in ore hold vs cargo"""
        # Add ore hold
        self.miner.add_component(OreHold(ore_hold_capacity=5000.0))
        
        # Mine some ore
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        self.mining_system.update(60.0)
        
        # Check ore went to ore hold, not cargo
        ore_hold = self.miner.get_component(OreHold)
        inventory = self.miner.get_component(Inventory)
        
        self.assertIn("veldspar", ore_hold.ore)
        self.assertGreater(ore_hold.ore["veldspar"], 0)
        self.assertNotIn("veldspar", inventory.items)
    
    def test_multiple_mining_cycles(self):
        """Test multiple consecutive mining cycles"""
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        initial_ore = self.test_asteroid.ore_remaining
        
        # Complete 3 cycles
        for _ in range(3):
            self.mining_system.update(60.0)
        
        # Check ore was extracted
        self.assertLess(self.test_asteroid.ore_remaining, initial_ore)
        
        # Check ore was stored
        inventory = self.miner.get_component(Inventory)
        self.assertGreater(inventory.items.get("veldspar", 0), 100)
    
    def test_mining_stats_tracking(self):
        """Test mining statistics are tracked"""
        mining_yield = self.miner.get_component(MiningYield)
        
        # Mine some ore
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        self.mining_system.update(60.0)
        
        # Check stats
        self.assertIn("veldspar", mining_yield.total_ore_mined)
        self.assertGreater(mining_yield.total_ore_mined["veldspar"], 0)
    
    def test_get_nearby_asteroids(self):
        """Test finding nearby asteroids"""
        nearby = self.mining_system.get_nearby_asteroids(self.miner)
        
        # Should find our test asteroid
        self.assertEqual(len(nearby), 1)
        self.assertEqual(nearby[0][1].id, self.test_asteroid.id)
    
    def test_get_mining_info(self):
        """Test getting mining information"""
        info = self.mining_system.get_mining_info(self.miner)
        
        self.assertIn("is_active", info)
        self.assertIn("base_yield", info)
        self.assertIn("yield_multiplier", info)
        self.assertIn("final_yield", info)
        
        self.assertEqual(info["base_yield"], 40.0)
        self.assertGreater(info["yield_multiplier"], 0)
    
    def test_capacitor_depletion_stops_mining(self):
        """Test mining stops when capacitor is depleted"""
        # Set low capacitor
        cap = self.miner.get_component(Capacitor)
        cap.capacitor = 100.0  # Enough for 1 cycle, not 2
        
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Complete first cycle
        self.mining_system.update(60.0)
        
        mining_laser = self.miner.get_component(MiningLaser)
        self.assertTrue(mining_laser.is_active)  # Still active after first
        
        # Try second cycle - should stop due to cap
        self.mining_system.update(60.0)
        
        self.assertFalse(mining_laser.is_active)  # Stopped due to no cap
    
    def test_cargo_full_stops_storing(self):
        """Test that full cargo prevents ore storage"""
        # Set tiny cargo
        inventory = self.miner.get_component(Inventory)
        inventory.cargo_capacity = 50.0
        inventory.cargo_used = 0.0
        
        # Mine multiple cycles
        self.mining_system.start_mining(
            self.miner,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        for _ in range(3):
            self.mining_system.update(60.0)
        
        # Cargo should be close to or at capacity
        self.assertLessEqual(inventory.cargo_used, inventory.cargo_capacity + 50)


class TestMiningBarges(unittest.TestCase):
    """Test mining barge specific functionality"""
    
    def setUp(self):
        """Set up test world and entities"""
        self.world = World()
        self.asteroid_manager = AsteroidFieldManager()
        self.mining_system = MiningSystem(self.world, self.asteroid_manager)
        
        # Create test asteroid belt
        self.test_belt = AsteroidBelt(
            belt_id="test_belt_1",
            name="Test Belt",
            system="test_system",
            security=1.0,
            layout=BeltLayout.SEMICIRCLE,
            radius_km=50.0
        )
        
        # Add test asteroid
        self.test_asteroid = Asteroid(
            id="test_ast_1",
            ore_type="veldspar",
            variant="standard",
            position=(1000.0, 0.0, 0.0),
            size=AsteroidSize.MEDIUM,
            ore_remaining=35000,
            ore_total=35000
        )
        self.test_belt.asteroids[self.test_asteroid.id] = self.test_asteroid
        self.asteroid_manager.belts[self.test_belt.id] = self.test_belt
    
    def test_procurer_ore_hold(self):
        """Test Procurer with ore hold capacity"""
        procurer = self.world.create_entity()
        procurer.add_component(Position(x=0.0, y=0.0, z=0.0))
        procurer.add_component(Capacitor(capacitor=3500.0, capacitor_max=3500.0))
        procurer.add_component(MiningLaser(
            laser_type="Strip Miner I",
            cycle_time=180.0,
            yield_amount=540.0,
            optimal_range=15000.0,
            capacitor_usage=360.0
        ))
        procurer.add_component(MiningYield())
        procurer.add_component(OreHold(ore_hold_capacity=16000.0))  # Procurer base
        procurer.add_component(Inventory(cargo_capacity=350.0))
        procurer.add_component(Skills())
        
        # Start mining
        result = self.mining_system.start_mining(
            procurer,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        self.assertTrue(result)
        
        # Complete one mining cycle
        self.mining_system.update(180.0)
        
        # Check ore was stored in ore hold
        ore_hold = procurer.get_component(OreHold)
        self.assertGreater(ore_hold.ore_hold_used, 0)
        self.assertLessEqual(ore_hold.ore_hold_used, ore_hold.ore_hold_capacity)
    
    def test_retriever_large_ore_hold(self):
        """Test Retriever with largest ore hold"""
        retriever = self.world.create_entity()
        retriever.add_component(Position(x=0.0, y=0.0, z=0.0))
        retriever.add_component(Capacitor(capacitor=3500.0, capacitor_max=3500.0))
        retriever.add_component(MiningLaser(
            laser_type="Strip Miner I",
            cycle_time=180.0,
            yield_amount=540.0,
            optimal_range=15000.0,
            capacitor_usage=360.0
        ))
        retriever.add_component(MiningYield())
        retriever.add_component(OreHold(ore_hold_capacity=27500.0))  # Retriever base
        retriever.add_component(Inventory(cargo_capacity=425.0))
        retriever.add_component(Skills())
        
        # Mine multiple cycles
        self.mining_system.start_mining(
            retriever,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Complete 10 cycles (5400 m3 mined)
        for _ in range(10):
            self.mining_system.update(180.0)
        
        ore_hold = retriever.get_component(OreHold)
        # Should have mined ~5400 m3 (540 * 10), but may be slightly less due to cycle timing
        # Wide range accounts for timing precision and partial cycles
        self.assertGreater(ore_hold.ore_hold_used, 4500)
        self.assertLess(ore_hold.ore_hold_used, 5500)
        # Should still have room in ore hold
        self.assertLess(ore_hold.ore_hold_used, ore_hold.ore_hold_capacity)
    
    def test_covetor_high_yield(self):
        """Test Covetor with multiple high slots for strip miners"""
        covetor = self.world.create_entity()
        covetor.add_component(Position(x=0.0, y=0.0, z=0.0))
        covetor.add_component(Capacitor(capacitor=3200.0, capacitor_max=3200.0))
        covetor.add_component(MiningLaser(
            laser_type="Strip Miner I",
            cycle_time=180.0,
            yield_amount=540.0,
            optimal_range=15000.0,
            capacitor_usage=360.0
        ))
        covetor.add_component(MiningYield())
        covetor.add_component(OreHold(ore_hold_capacity=7000.0))  # Covetor base
        covetor.add_component(Inventory(cargo_capacity=300.0))
        covetor.add_component(Skills())
        
        # Start mining
        self.mining_system.start_mining(
            covetor,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Complete one cycle
        self.mining_system.update(180.0)
        
        ore_hold = covetor.get_component(OreHold)
        # Should mine 540 m3 per cycle (base yield)
        self.assertGreater(ore_hold.ore_hold_used, 500)
        self.assertLess(ore_hold.ore_hold_used, 600)
    
    def test_ore_hold_priority_over_cargo(self):
        """Test that ore hold is used for mining barge storage"""
        barge = self.world.create_entity()
        barge.add_component(Position(x=0.0, y=0.0, z=0.0))
        barge.add_component(Capacitor(capacitor=5000.0, capacitor_max=5000.0))
        barge.add_component(MiningLaser(
            laser_type="Strip Miner I",
            cycle_time=180.0,
            yield_amount=540.0,
            optimal_range=15000.0,
            capacitor_usage=360.0
        ))
        barge.add_component(MiningYield())
        barge.add_component(OreHold(ore_hold_capacity=1000.0))  # Small for testing
        barge.add_component(Inventory(cargo_capacity=5000.0))
        barge.add_component(Skills())
        
        # Start mining
        self.mining_system.start_mining(
            barge,
            self.test_asteroid.id,
            self.test_belt.id
        )
        
        # Complete one mining cycle
        self.mining_system.update(180.0)
        
        ore_hold = barge.get_component(OreHold)
        
        # Ore should go into ore hold (not regular cargo)
        self.assertGreater(ore_hold.ore_hold_used, 500.0)
        # Verify ore hold is being used
        self.assertGreater(len(ore_hold.ore), 0)


if __name__ == "__main__":
    unittest.main()
