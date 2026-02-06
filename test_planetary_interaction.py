"""
Test Planetary Interaction (PI) System
Tests colony creation, resource extraction, and material processing
"""

import unittest
from engine.core.ecs import World
from engine.components.game_components import (
    PlanetaryColony, PIStructure, PIResource, ExtractorQueue,
    Player, Inventory, Skills
)
from engine.systems.planetary_interaction_system import PlanetaryInteractionSystem


class TestPlanetaryInteractionSystem(unittest.TestCase):
    """Test planetary interaction mechanics"""
    
    def setUp(self):
        """Set up test world and systems"""
        self.world = World()
        self.world.game_time = 0.0  # Initialize game time
        self.pi_system = PlanetaryInteractionSystem(self.world)
        self.world.add_system(self.pi_system)
        
        # Create test player
        self.player = self.world.create_entity()
        self.player.add_component(Player(
            player_id="test_player_1",
            character_name="Test Pilot"
        ))
        self.player.add_component(Inventory())
        self.player.add_component(Skills())
    
    def test_colony_creation(self):
        """Test creating a planetary colony"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_001",
            planet_type="barren"
        )
        
        self.assertIsNotNone(colony_id)
        
        # Verify colony entity exists
        colony_entity = self.world.get_entity(colony_id)
        self.assertIsNotNone(colony_entity)
        
        # Verify colony component
        colony = colony_entity.get_component(PlanetaryColony)
        self.assertIsNotNone(colony)
        self.assertEqual(colony.planet_id, "planet_001")
        self.assertEqual(colony.planet_type, "barren")
        self.assertEqual(colony.owner_id, "test_player_1")
        
        # Verify command center was created
        self.assertIsNotNone(colony.command_center_id)
    
    def test_colony_resources(self):
        """Test colony CPU and powergrid tracking"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_002",
            planet_type="temperate"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        colony = colony_entity.get_component(PlanetaryColony)
        
        # Check initial resources
        self.assertEqual(colony.cpu_used, 0.0)
        self.assertEqual(colony.cpu_max, 1675.0)
        self.assertEqual(colony.powergrid_used, 0.0)
        self.assertEqual(colony.powergrid_max, 6000.0)
    
    def test_place_extractor(self):
        """Test placing an extractor on a planet"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_003",
            planet_type="lava"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place extractor
        extractor_id = self.pi_system.place_extractor(
            colony_entity,
            planet_x=0.3,
            planet_y=0.4,
            resource_id="base_metals"
        )
        
        self.assertIsNotNone(extractor_id)
        
        # Verify extractor entity
        extractor_entity = self.world.get_entity(extractor_id)
        self.assertIsNotNone(extractor_entity)
        
        # Verify extractor component
        extractor = extractor_entity.get_component(PIStructure)
        self.assertIsNotNone(extractor)
        self.assertEqual(extractor.structure_type, "extractor")
        self.assertEqual(extractor.planet_x, 0.3)
        self.assertEqual(extractor.planet_y, 0.4)
        self.assertEqual(extractor.extraction_resource, "base_metals")
        
        # Verify extraction queue
        queue = extractor_entity.get_component(ExtractorQueue)
        self.assertIsNotNone(queue)
        self.assertEqual(queue.resource_id, "base_metals")
        
        # Verify colony resources updated
        colony = colony_entity.get_component(PlanetaryColony)
        self.assertEqual(colony.cpu_used, 400.0)
        self.assertEqual(colony.powergrid_used, 1100.0)
        self.assertIn(extractor_id, colony.installations)
    
    def test_extractor_cpu_limit(self):
        """Test that extractors respect CPU limits"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_004",
            planet_type="gas"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        colony = colony_entity.get_component(PlanetaryColony)
        
        # Try to place too many extractors (CPU limit)
        extractors_placed = 0
        for i in range(10):
            extractor_id = self.pi_system.place_extractor(
                colony_entity,
                planet_x=0.1 * i,
                planet_y=0.1 * i,
                resource_id="noble_gas"
            )
            if extractor_id:
                extractors_placed += 1
        
        # Should only place 4 extractors (1675 CPU / 400 CPU per extractor = 4.18)
        self.assertEqual(extractors_placed, 4)
        self.assertLessEqual(colony.cpu_used, colony.cpu_max)
    
    def test_start_extraction_program(self):
        """Test starting an extraction program"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_005",
            planet_type="ice"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place extractor
        extractor_id = self.pi_system.place_extractor(
            colony_entity,
            planet_x=0.5,
            planet_y=0.5,
            resource_id="aqueous_liquids"
        )
        
        extractor_entity = self.world.get_entity(extractor_id)
        
        # Start extraction program for 24 hours
        success = self.pi_system.start_extraction_program(
            extractor_entity,
            duration_hours=24.0
        )
        
        self.assertTrue(success)
        
        # Verify extraction program
        extractor = extractor_entity.get_component(PIStructure)
        queue = extractor_entity.get_component(ExtractorQueue)
        
        self.assertTrue(extractor.is_active)
        self.assertEqual(queue.program_duration, 24.0 * 3600)
        self.assertEqual(queue.total_cycles, 24)  # 24 one-hour cycles
        self.assertGreater(queue.extraction_yield_per_cycle, 0)
    
    def test_extraction_cycle(self):
        """Test completing an extraction cycle"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_006",
            planet_type="storm"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place and start extractor
        extractor_id = self.pi_system.place_extractor(
            colony_entity,
            planet_x=0.5,
            planet_y=0.5,
            resource_id="suspended_plasma"
        )
        
        extractor_entity = self.world.get_entity(extractor_id)
        self.pi_system.start_extraction_program(extractor_entity, duration_hours=1.0)
        
        extractor = extractor_entity.get_component(PIStructure)
        queue = extractor_entity.get_component(ExtractorQueue)
        
        # Simulate one full extraction cycle (1 hour = 3600 seconds)
        self.world.game_time = 3600.0
        self.pi_system.update(3600.0)
        
        # Verify extraction occurred
        self.assertEqual(queue.cycles_completed, 1)
        self.assertGreater(queue.total_extracted, 0)
        self.assertIn("suspended_plasma", extractor.storage)
        self.assertGreater(extractor.storage["suspended_plasma"], 0)
    
    def test_place_processor(self):
        """Test placing a processor facility"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_007",
            planet_type="oceanic"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place basic processor
        processor_id = self.pi_system.place_processor(
            colony_entity,
            planet_x=0.6,
            planet_y=0.6,
            processor_type="basic_processor"
        )
        
        self.assertIsNotNone(processor_id)
        
        # Verify processor
        processor_entity = self.world.get_entity(processor_id)
        processor = processor_entity.get_component(PIStructure)
        
        self.assertEqual(processor.structure_type, "basic_processor")
        self.assertEqual(processor.cpu_usage, 200.0)
        self.assertEqual(processor.powergrid_usage, 800.0)
        
        # Verify colony resources
        colony = colony_entity.get_component(PlanetaryColony)
        self.assertEqual(colony.cpu_used, 200.0)
        self.assertEqual(colony.powergrid_used, 800.0)
    
    def test_set_processor_recipe(self):
        """Test setting a processor recipe"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_008",
            planet_type="plasma"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place processor
        processor_id = self.pi_system.place_processor(
            colony_entity,
            planet_x=0.7,
            planet_y=0.7,
            processor_type="basic_processor"
        )
        
        processor_entity = self.world.get_entity(processor_id)
        
        # Set recipe
        success = self.pi_system.set_processor_recipe(
            processor_entity,
            recipe_id="water"
        )
        
        self.assertTrue(success)
        
        # Verify recipe set
        processor = processor_entity.get_component(PIStructure)
        self.assertEqual(processor.recipe_id, "water")
        self.assertTrue(processor.is_active)
    
    def test_place_launchpad(self):
        """Test placing a launchpad"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_009",
            planet_type="temperate"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place launchpad
        launchpad_id = self.pi_system.place_launchpad(
            colony_entity,
            planet_x=0.8,
            planet_y=0.8
        )
        
        self.assertIsNotNone(launchpad_id)
        
        # Verify launchpad
        launchpad_entity = self.world.get_entity(launchpad_id)
        launchpad = launchpad_entity.get_component(PIStructure)
        
        self.assertEqual(launchpad.structure_type, "launchpad")
        self.assertEqual(launchpad.cpu_usage, 510.0)
        self.assertEqual(launchpad.powergrid_usage, 700.0)
        self.assertTrue(launchpad.is_active)
        
        # Verify colony resources
        colony = colony_entity.get_component(PlanetaryColony)
        self.assertEqual(colony.cpu_used, 510.0)
        self.assertEqual(colony.powergrid_used, 700.0)
    
    def test_transfer_materials(self):
        """Test transferring materials between structures"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_010",
            planet_type="barren"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place extractor and launchpad
        extractor_id = self.pi_system.place_extractor(
            colony_entity, 0.3, 0.3, "base_metals"
        )
        launchpad_id = self.pi_system.place_launchpad(
            colony_entity, 0.7, 0.7
        )
        
        extractor_entity = self.world.get_entity(extractor_id)
        launchpad_entity = self.world.get_entity(launchpad_id)
        
        # Start extraction
        self.pi_system.start_extraction_program(extractor_entity, duration_hours=1.0)
        
        # Simulate extraction
        self.world.game_time = 3600.0
        self.pi_system.update(3600.0)
        
        # Get extracted amount
        extractor = extractor_entity.get_component(PIStructure)
        extracted_amount = extractor.storage.get("base_metals", 0)
        self.assertGreater(extracted_amount, 0)
        
        # Transfer to launchpad
        success = self.pi_system.transfer_materials(
            extractor_entity,
            launchpad_entity,
            "base_metals",
            extracted_amount
        )
        
        self.assertTrue(success)
        
        # Verify transfer
        extractor = extractor_entity.get_component(PIStructure)
        launchpad = launchpad_entity.get_component(PIStructure)
        
        self.assertEqual(extractor.storage.get("base_metals", 0), 0)
        self.assertEqual(launchpad.storage.get("base_metals", 0), extracted_amount)
    
    def test_export_from_launchpad(self):
        """Test exporting materials from launchpad to player inventory"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_011",
            planet_type="lava"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place launchpad
        launchpad_id = self.pi_system.place_launchpad(
            colony_entity, 0.5, 0.5
        )
        
        launchpad_entity = self.world.get_entity(launchpad_id)
        launchpad = launchpad_entity.get_component(PIStructure)
        
        # Add some materials to launchpad
        launchpad.storage["heavy_metals"] = 1000
        
        # Export to player
        success = self.pi_system.export_from_launchpad(
            launchpad_entity,
            self.player,
            "heavy_metals",
            1000
        )
        
        self.assertTrue(success)
        
        # Verify export
        launchpad = launchpad_entity.get_component(PIStructure)
        inventory = self.player.get_component(Inventory)
        
        self.assertEqual(launchpad.storage.get("heavy_metals", 0), 0)
        self.assertEqual(inventory.items.get("heavy_metals", 0), 1000)
    
    def test_get_colony_info(self):
        """Test getting colony information"""
        colony_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_012",
            planet_type="ice"
        )
        
        colony_entity = self.world.get_entity(colony_id)
        
        # Place some structures
        self.pi_system.place_extractor(colony_entity, 0.2, 0.2, "aqueous_liquids")
        self.pi_system.place_extractor(colony_entity, 0.4, 0.4, "noble_gas")
        self.pi_system.place_processor(colony_entity, 0.6, 0.6, "basic_processor")
        self.pi_system.place_launchpad(colony_entity, 0.8, 0.8)
        
        # Get colony info
        info = self.pi_system.get_colony_info(colony_entity)
        
        self.assertEqual(info["planet_id"], "planet_012")
        self.assertEqual(info["planet_type"], "ice")
        self.assertEqual(info["total_installations"], 4)
        self.assertEqual(info["installations"]["extractor"], 2)
        self.assertEqual(info["installations"]["basic_processor"], 1)
        self.assertEqual(info["installations"]["launchpad"], 1)
        self.assertGreater(info["cpu_used"], 0)
        self.assertGreater(info["powergrid_used"], 0)
    
    def test_multiple_colonies(self):
        """Test player can have multiple colonies"""
        # Create first colony
        colony1_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_013",
            planet_type="barren"
        )
        
        # Create second colony
        colony2_id = self.pi_system.create_colony(
            self.player,
            planet_id="planet_014",
            planet_type="temperate"
        )
        
        self.assertIsNotNone(colony1_id)
        self.assertIsNotNone(colony2_id)
        self.assertNotEqual(colony1_id, colony2_id)
        
        # Verify both colonies exist
        colony1_entity = self.world.get_entity(colony1_id)
        colony2_entity = self.world.get_entity(colony2_id)
        
        self.assertIsNotNone(colony1_entity)
        self.assertIsNotNone(colony2_entity)
        
        # Verify different planets
        colony1 = colony1_entity.get_component(PlanetaryColony)
        colony2 = colony2_entity.get_component(PlanetaryColony)
        
        self.assertNotEqual(colony1.planet_id, colony2.planet_id)


if __name__ == '__main__':
    # Run tests
    unittest.main()
