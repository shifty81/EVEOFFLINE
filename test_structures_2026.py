#!/usr/bin/env python3
"""
Tests for EVE 2026 structure data files.
Validates that structure JSON files are well-formed and contain expected data.
"""

import json
import os
import unittest


class TestStructureData(unittest.TestCase):
    """Test cases for structure data files."""

    def setUp(self):
        """Set up test fixtures."""
        # Get the directory where this test file is located
        test_dir = os.path.dirname(os.path.abspath(__file__))
        self.data_dir = os.path.join(test_dir, 'data', 'universe')
        self.player_structures_file = os.path.join(self.data_dir, 'player_structures.json')
        self.station_types_file = os.path.join(self.data_dir, 'station_types.json')

    def test_player_structures_file_exists(self):
        """Test that player_structures.json exists."""
        self.assertTrue(os.path.exists(self.player_structures_file),
                       "player_structures.json should exist")

    def test_player_structures_valid_json(self):
        """Test that player_structures.json is valid JSON."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        self.assertIsInstance(data, dict, "player_structures.json should contain a dictionary")

    def test_player_structures_has_main_sections(self):
        """Test that player_structures.json has expected main sections."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        self.assertIn('player_structures_2026', data)
        structures = data['player_structures_2026']
        
        # Check for main categories
        self.assertIn('nullsec_strategic', structures)
        self.assertIn('mining_industrial', structures)
        self.assertIn('wormhole_structures', structures)
        self.assertIn('other_deployables', structures)
        self.assertIn('structure_summary_table', structures)
        self.assertIn('design_philosophy_2026', structures)

    def test_orbital_skyhook_structure(self):
        """Test that Orbital Skyhook structure has required fields."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        skyhook = data['player_structures_2026']['nullsec_strategic']['orbital_skyhook']
        
        # Check required fields
        self.assertEqual(skyhook['name'], 'Orbital Skyhook')
        self.assertEqual(skyhook['type'], 'nullsec_structure')
        self.assertIn('description', skyhook)
        self.assertIn('deployment', skyhook)
        self.assertIn('functionality', skyhook)
        self.assertIn('defense', skyhook)
        self.assertIn('strategic_value', skyhook)

    def test_sovereignty_hub_structure(self):
        """Test that Sovereignty Hub structure has required fields."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        sov_hub = data['player_structures_2026']['nullsec_strategic']['sovereignty_hub']
        
        # Check required fields
        self.assertEqual(sov_hub['name'], 'Sovereignty Hub (Sov Hub)')
        self.assertEqual(sov_hub['type'], 'nullsec_structure')
        self.assertIn('upgrades', sov_hub)
        self.assertIn('industrial', sov_hub['upgrades'])
        self.assertIn('military', sov_hub['upgrades'])
        self.assertIn('economic', sov_hub['upgrades'])

    def test_metenox_moon_drill_structure(self):
        """Test that Metenox Moon Drill structure has required fields."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        metenox = data['player_structures_2026']['nullsec_strategic']['metenox_moon_drill']
        
        # Check required fields
        self.assertEqual(metenox['name'], 'Metenox Moon Drill')
        self.assertEqual(metenox['category'], 'moon_mining')
        self.assertIn('advantages', metenox)
        self.assertIn('lower_cost', metenox['advantages'])

    def test_communal_granary_structure(self):
        """Test that Communal Granary structure has required fields."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        granary = data['player_structures_2026']['wormhole_structures']['communal_granary']
        
        # Check required fields
        self.assertEqual(granary['name'], 'Communal Granary')
        self.assertEqual(granary['type'], 'wormhole_structure')
        self.assertIn('unique_properties', granary)
        self.assertTrue(granary['unique_properties']['indestructible'])
        self.assertIn('buff_mechanism', granary['functionality'])

    def test_structure_summary_table(self):
        """Test that structure summary table has all expected structures."""
        with open(self.player_structures_file, 'r') as f:
            data = json.load(f)
        
        summary = data['player_structures_2026']['structure_summary_table']
        
        # Check table structure
        self.assertIn('columns', summary)
        self.assertIn('rows', summary)
        self.assertGreaterEqual(len(summary['rows']), 5, "Should have at least 5 structures")
        
        # Check for key structures
        structure_names = [row['structure'] for row in summary['rows']]
        self.assertIn('Orbital Skyhook', structure_names)
        self.assertIn('Sovereignty Hub', structure_names)
        self.assertIn('Metenox Moon Drill', structure_names)
        self.assertIn('Communal Granary', structure_names)

    def test_station_types_file_exists(self):
        """Test that station_types.json exists."""
        self.assertTrue(os.path.exists(self.station_types_file),
                       "station_types.json should exist")

    def test_station_types_valid_json(self):
        """Test that station_types.json is valid JSON."""
        with open(self.station_types_file, 'r') as f:
            data = json.load(f)
        self.assertIsInstance(data, dict, "station_types.json should contain a dictionary")

    def test_station_types_has_upwell_structures(self):
        """Test that station_types.json includes Upwell structures."""
        with open(self.station_types_file, 'r') as f:
            data = json.load(f)
        
        self.assertIn('upwell_structures', data)
        upwell = data['upwell_structures']
        
        # Check for main categories
        self.assertIn('citadels', upwell)
        self.assertIn('engineering_complexes', upwell)
        self.assertIn('refineries', upwell)
        self.assertIn('structure_features', upwell)

    def test_upwell_citadels(self):
        """Test that Upwell citadels are properly defined."""
        with open(self.station_types_file, 'r') as f:
            data = json.load(f)
        
        citadels = data['upwell_structures']['citadels']
        
        # Check for all citadel types
        self.assertIn('astrahus', citadels)
        self.assertIn('fortizar', citadels)
        self.assertIn('keepstar', citadels)
        
        # Check Keepstar specific features
        keepstar = citadels['keepstar']
        self.assertEqual(keepstar['docking_capacity'], 'all_ships_including_titans')
        self.assertIn('supercapital_docking', keepstar['special_features'])

    def test_upwell_refineries_2026_updates(self):
        """Test that refineries have 2026 updates noted."""
        with open(self.station_types_file, 'r') as f:
            data = json.load(f)
        
        refineries = data['upwell_structures']['refineries']
        
        # Check Athanor and Tatara have 2026 updates
        self.assertIn('updates_2026', refineries['athanor'])
        self.assertIn('updates_2026', refineries['tatara'])
        
        # Verify update content
        athanor_updates = refineries['athanor']['updates_2026']
        self.assertIn('fitting_system', athanor_updates)

    def test_structure_features(self):
        """Test that structure features are properly defined."""
        with open(self.station_types_file, 'r') as f:
            data = json.load(f)
        
        features = data['upwell_structures']['structure_features']
        
        # Check for key features
        self.assertIn('asset_safety', features)
        self.assertIn('access_control', features)
        self.assertIn('tax_rates', features)
        self.assertIn('fitting_system_2026', features)


def run_tests():
    """Run all tests and print results."""
    loader = unittest.TestLoader()
    suite = loader.loadTestsFromTestCase(TestStructureData)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
