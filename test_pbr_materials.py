#!/usr/bin/env python3
"""
Test suite for PBR Materials System
Tests PBR material creation and management
"""

import unittest
import tempfile
import shutil
from unittest.mock import Mock, patch, MagicMock
from panda3d.core import Vec3, Vec4


class TestPBRMaterial(unittest.TestCase):
    """Test PBRMaterial class"""
    
    def test_initialization(self):
        """Test material initialization"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test_material")
        
        self.assertEqual(mat.name, "test_material")
        self.assertEqual(mat.metallic, 0.0)
        self.assertEqual(mat.roughness, 0.5)
        self.assertIsNotNone(mat.material)
    
    def test_set_base_color(self):
        """Test setting base color"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        color = Vec4(1.0, 0.5, 0.0, 1.0)
        mat.set_base_color(color)
        
        self.assertEqual(mat.base_color, color)
    
    def test_set_metallic(self):
        """Test setting metallic value"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        mat.set_metallic(0.8)
        
        self.assertEqual(mat.metallic, 0.8)
        
        # Test clamping
        mat.set_metallic(1.5)
        self.assertEqual(mat.metallic, 1.0)
        
        mat.set_metallic(-0.5)
        self.assertEqual(mat.metallic, 0.0)
    
    def test_set_roughness(self):
        """Test setting roughness value"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        mat.set_roughness(0.3)
        
        self.assertEqual(mat.roughness, 0.3)
        
        # Test clamping
        mat.set_roughness(1.5)
        self.assertEqual(mat.roughness, 1.0)
        
        mat.set_roughness(-0.5)
        self.assertEqual(mat.roughness, 0.0)
    
    def test_set_emission(self):
        """Test setting emission"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        emission = Vec3(1.0, 0.5, 0.0)
        mat.set_emission(emission, 2.0)
        
        self.assertEqual(mat.emission, emission)
        self.assertEqual(mat.emission_strength, 2.0)
    
    def test_apply_to_node(self):
        """Test applying material to node"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        mat.set_base_color(Vec4(1.0, 0.0, 0.0, 1.0))
        
        mock_node = Mock()
        mock_node.setMaterial = Mock()
        mock_node.setTexture = Mock()
        
        mat.apply_to_node(mock_node)
        
        # Check that setMaterial was called
        mock_node.setMaterial.assert_called_once()


class TestPBRMaterialLibrary(unittest.TestCase):
    """Test PBRMaterialLibrary class"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_loader = Mock()
        self.test_dir = tempfile.mkdtemp()
    
    def tearDown(self):
        """Clean up test files"""
        if hasattr(self, 'test_dir'):
            shutil.rmtree(self.test_dir, ignore_errors=True)
    
    def test_initialization(self):
        """Test library initialization"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        # Should have default materials
        self.assertGreater(len(library.materials), 0)
        self.assertIsNotNone(library.loader)
    
    def test_default_materials(self):
        """Test that default materials are created"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        # Check for some expected default materials
        self.assertIn('steel', library.materials)
        self.assertIn('gold', library.materials)
        self.assertIn('ship_hull_minmatar', library.materials)
        self.assertIn('ship_hull_caldari', library.materials)
        self.assertIn('ship_hull_gallente', library.materials)
        self.assertIn('ship_hull_amarr', library.materials)
        self.assertIn('engine_glow_blue', library.materials)
    
    def test_get_material(self):
        """Test getting material by name"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        # Get existing material
        steel = library.get_material('steel')
        self.assertIsNotNone(steel)
        self.assertEqual(steel.name, 'steel')
        
        # Get non-existent material
        nonexistent = library.get_material('nonexistent')
        self.assertIsNone(nonexistent)
    
    def test_create_material(self):
        """Test creating custom material"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        mat = library.create_material(
            'custom',
            base_color=Vec4(1.0, 0.0, 0.0, 1.0),
            metallic=0.5,
            roughness=0.3
        )
        
        self.assertIsNotNone(mat)
        self.assertEqual(mat.name, 'custom')
        self.assertEqual(mat.metallic, 0.5)
        self.assertEqual(mat.roughness, 0.3)
        
        # Check it's in the library
        self.assertIn('custom', library.materials)
    
    def test_apply_material_to_node(self):
        """Test applying material to node"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        mock_node = Mock()
        mock_node.setMaterial = Mock()
        
        library.apply_material_to_node(mock_node, 'steel')
        
        # Material should be applied
        mock_node.setMaterial.assert_called()
    
    def test_list_materials(self):
        """Test listing all materials"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        materials = library.list_materials()
        
        self.assertIsInstance(materials, list)
        self.assertGreater(len(materials), 0)
        self.assertIn('steel', materials)
    
    def test_get_faction_material(self):
        """Test getting faction-specific materials"""
        from client_3d.rendering.pbr_materials import PBRMaterialLibrary
        
        library = PBRMaterialLibrary(self.mock_loader, self.test_dir)
        
        # Test known factions
        minmatar = library.get_faction_material('Minmatar')
        self.assertIsNotNone(minmatar)
        self.assertEqual(minmatar.name, 'ship_hull_minmatar')
        
        caldari = library.get_faction_material('Caldari')
        self.assertIsNotNone(caldari)
        self.assertEqual(caldari.name, 'ship_hull_caldari')
        
        # Test unknown faction (should return default)
        unknown = library.get_faction_material('Unknown')
        self.assertIsNotNone(unknown)


class TestPBRShaderManager(unittest.TestCase):
    """Test PBRShaderManager class"""
    
    def test_initialization(self):
        """Test shader manager initialization"""
        from client_3d.rendering.pbr_materials import PBRShaderManager
        
        manager = PBRShaderManager()
        
        self.assertIsNotNone(manager)
        self.assertEqual(len(manager.shaders), 0)
    
    def test_apply_pbr_shader_fallback(self):
        """Test applying shader with fallback to auto-shader"""
        from client_3d.rendering.pbr_materials import PBRShaderManager
        
        manager = PBRShaderManager()
        
        mock_node = Mock()
        mock_node.setShader = Mock()
        mock_node.setShaderAuto = Mock()
        
        # Should fall back to auto-shader if custom shader not found
        manager.apply_pbr_shader(mock_node, "nonexistent")
        
        # Auto-shader should be called
        mock_node.setShaderAuto.assert_called()


class TestPBRIntegration(unittest.TestCase):
    """Test PBR integration with renderer"""
    
    def test_renderer_has_material_library(self):
        """Test that renderer has PBR material library"""
        from client_3d.rendering.renderer import EntityRenderer
        from unittest.mock import Mock
        
        mock_render = Mock()
        mock_loader = Mock()
        
        renderer = EntityRenderer(mock_render, mock_loader)
        
        # Check material library exists
        self.assertTrue(hasattr(renderer, 'material_library'))
        self.assertIsNotNone(renderer.material_library)


class TestMaterialProperties(unittest.TestCase):
    """Test material property ranges and validation"""
    
    def test_metallic_range(self):
        """Test metallic value stays in 0-1 range"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        
        # Test valid values
        for value in [0.0, 0.5, 1.0]:
            mat.set_metallic(value)
            self.assertEqual(mat.metallic, value)
        
        # Test clamping
        mat.set_metallic(2.0)
        self.assertEqual(mat.metallic, 1.0)
        
        mat.set_metallic(-1.0)
        self.assertEqual(mat.metallic, 0.0)
    
    def test_roughness_range(self):
        """Test roughness value stays in 0-1 range"""
        from client_3d.rendering.pbr_materials import PBRMaterial
        
        mat = PBRMaterial("test")
        
        # Test valid values
        for value in [0.0, 0.5, 1.0]:
            mat.set_roughness(value)
            self.assertEqual(mat.roughness, value)
        
        # Test clamping
        mat.set_roughness(2.0)
        self.assertEqual(mat.roughness, 1.0)
        
        mat.set_roughness(-1.0)
        self.assertEqual(mat.roughness, 0.0)


def run_tests():
    """Run all tests"""
    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestPBRMaterial))
    suite.addTests(loader.loadTestsFromTestCase(TestPBRMaterialLibrary))
    suite.addTests(loader.loadTestsFromTestCase(TestPBRShaderManager))
    suite.addTests(loader.loadTestsFromTestCase(TestPBRIntegration))
    suite.addTests(loader.loadTestsFromTestCase(TestMaterialProperties))
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
