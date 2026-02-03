#!/usr/bin/env python3
"""
Test suite for Asset Pipeline
Tests loading of external 3D models and asset management
"""

import unittest
import os
import tempfile
import shutil
from unittest.mock import Mock, patch, MagicMock


class TestAssetLoader(unittest.TestCase):
    """Test AssetLoader functionality"""
    
    def setUp(self):
        """Set up test fixtures"""
        # Create temporary directory for test assets
        self.test_dir = tempfile.mkdtemp()
        self.assets_dir = os.path.join(self.test_dir, "assets")
        
        # Mock Panda3D loader
        self.mock_loader = Mock()
        
    def tearDown(self):
        """Clean up test files"""
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)
    
    def test_initialization(self):
        """Test AssetLoader initialization"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Check directories were created
        self.assertTrue(os.path.exists(self.assets_dir))
        self.assertTrue(os.path.exists(os.path.join(self.assets_dir, "models")))
        self.assertTrue(os.path.exists(os.path.join(self.assets_dir, "textures")))
        self.assertTrue(os.path.exists(os.path.join(self.assets_dir, "ships")))
        
        # Check initialization
        self.assertEqual(loader.assets_dir, self.assets_dir)
        self.assertEqual(len(loader.model_cache), 0)
        self.assertEqual(len(loader.texture_cache), 0)
    
    def test_supported_formats(self):
        """Test supported file formats"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Check supported formats
        self.assertIn('.obj', loader.SUPPORTED_FORMATS)
        self.assertIn('.gltf', loader.SUPPORTED_FORMATS)
        self.assertIn('.glb', loader.SUPPORTED_FORMATS)
        self.assertIn('.bam', loader.SUPPORTED_FORMATS)
        self.assertIn('.egg', loader.SUPPORTED_FORMATS)
    
    def test_resolve_path(self):
        """Test path resolution"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Create test file
        test_file = os.path.join(self.assets_dir, "models", "test.obj")
        os.makedirs(os.path.dirname(test_file), exist_ok=True)
        with open(test_file, 'w') as f:
            f.write("# test obj file")
        
        # Test resolution
        resolved = loader._resolve_path("test.obj")
        self.assertIsNotNone(resolved)
        self.assertTrue(os.path.exists(resolved))
    
    def test_load_model_not_found(self):
        """Test loading non-existent model"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Try to load non-existent model
        model = loader.load_model("nonexistent.obj")
        self.assertIsNone(model)
    
    def test_load_model_unsupported_format(self):
        """Test loading unsupported format"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Create test file with unsupported format
        test_file = os.path.join(self.assets_dir, "models", "test.xyz")
        with open(test_file, 'w') as f:
            f.write("test")
        
        # Try to load
        model = loader.load_model("test.xyz")
        self.assertIsNone(model)
    
    def test_cache_functionality(self):
        """Test model caching"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Create test file
        test_file = os.path.join(self.assets_dir, "models", "test.bam")
        with open(test_file, 'w') as f:
            f.write("test")
        
        # Mock successful load
        mock_model = Mock()
        mock_model.copyTo = Mock(return_value=Mock())
        self.mock_loader.loadModel = Mock(return_value=mock_model)
        
        # Load model (should cache it)
        model1 = loader.load_model("test.bam", cache=True)
        
        # Check cache
        self.assertEqual(len(loader.model_cache), 1)
        self.assertIn("test.bam", loader.model_cache)
        
        # Load again (should use cache)
        model2 = loader.load_model("test.bam", cache=True)
        
        # Should only load once from disk
        self.assertEqual(self.mock_loader.loadModel.call_count, 1)
    
    def test_clear_cache(self):
        """Test cache clearing"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Add some cache entries
        loader.model_cache["test1"] = Mock()
        loader.model_cache["test2"] = Mock()
        loader.texture_cache["tex1"] = "path1"
        
        # Clear cache
        loader.clear_cache()
        
        # Check empty
        self.assertEqual(len(loader.model_cache), 0)
        self.assertEqual(len(loader.texture_cache), 0)
    
    def test_get_cache_info(self):
        """Test cache info retrieval"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Add cache entries
        loader.model_cache["test1"] = Mock()
        loader.model_cache["test2"] = Mock()
        loader.texture_cache["tex1"] = "path1"
        
        # Get info
        info = loader.get_cache_info()
        
        self.assertEqual(info['models_cached'], 2)
        self.assertEqual(info['textures_cached'], 1)
    
    def test_list_available_models(self):
        """Test listing available models"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Create test files
        models_dir = os.path.join(self.assets_dir, "models")
        ships_dir = os.path.join(self.assets_dir, "ships")
        
        # Create model files
        with open(os.path.join(models_dir, "test1.obj"), 'w') as f:
            f.write("test")
        with open(os.path.join(models_dir, "test2.gltf"), 'w') as f:
            f.write("test")
        with open(os.path.join(ships_dir, "frigate.bam"), 'w') as f:
            f.write("test")
        
        # Create non-model file (should be ignored)
        with open(os.path.join(models_dir, "readme.txt"), 'w') as f:
            f.write("test")
        
        # List models
        models = loader.list_available_models()
        
        # Check results
        self.assertEqual(len(models), 3)
        self.assertIn("test1.obj", models)
        self.assertIn("test2.gltf", models)
        self.assertTrue(any("frigate.bam" in m for m in models))
    
    def test_save_asset_manifest(self):
        """Test saving asset manifest"""
        from client_3d.rendering.asset_loader import AssetLoader
        
        loader = AssetLoader(self.mock_loader, self.assets_dir)
        
        # Create test model
        models_dir = os.path.join(self.assets_dir, "models")
        with open(os.path.join(models_dir, "test.obj"), 'w') as f:
            f.write("test")
        
        # Save manifest
        manifest_path = os.path.join(self.test_dir, "manifest.json")
        loader.save_asset_manifest(manifest_path)
        
        # Check file exists
        self.assertTrue(os.path.exists(manifest_path))
        
        # Read and validate
        import json
        with open(manifest_path, 'r') as f:
            manifest = json.load(f)
        
        self.assertIn('models', manifest)
        self.assertIn('cache_info', manifest)
        self.assertIn('supported_formats', manifest)
        self.assertEqual(len(manifest['models']), 1)


class TestAssetLoaderIntegration(unittest.TestCase):
    """Test AssetLoader integration with renderer"""
    
    def test_renderer_integration(self):
        """Test that renderer can use AssetLoader"""
        from client_3d.rendering.renderer import EntityRenderer
        from unittest.mock import Mock
        
        # Mock Panda3D components
        mock_render = Mock()
        mock_loader = Mock()
        
        # Create renderer
        renderer = EntityRenderer(mock_render, mock_loader)
        
        # Check AssetLoader exists
        self.assertTrue(hasattr(renderer, 'asset_loader'))
        self.assertIsNotNone(renderer.asset_loader)


class TestAssetValidation(unittest.TestCase):
    """Test model validation and optimization"""
    
    def test_validate_model(self):
        """Test model validation"""
        from client_3d.rendering.asset_loader import AssetLoader
        from unittest.mock import Mock
        
        loader = AssetLoader(Mock(), tempfile.mkdtemp())
        
        # Create mock model
        mock_model = Mock()
        mock_model.isEmpty = Mock(return_value=False)
        mock_model.getBounds = Mock(return_value="Bounds")
        mock_model.flattenStrong = Mock()
        mock_model.setShaderAuto = Mock()
        
        # Validate
        validated = loader._validate_model(mock_model, "test.obj")
        
        # Check methods were called
        mock_model.flattenStrong.assert_called_once()
        mock_model.setShaderAuto.assert_called_once()
        
        self.assertIsNotNone(validated)
    
    def test_validate_empty_model(self):
        """Test validation of empty model"""
        from client_3d.rendering.asset_loader import AssetLoader
        from unittest.mock import Mock
        
        loader = AssetLoader(Mock(), tempfile.mkdtemp())
        
        # Create empty mock model
        mock_model = Mock()
        mock_model.isEmpty = Mock(return_value=True)
        
        # Validate
        validated = loader._validate_model(mock_model, "test.obj")
        
        # Should still return model (just log warning)
        self.assertIsNotNone(validated)


def run_tests():
    """Run all tests"""
    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestAssetLoader))
    suite.addTests(loader.loadTestsFromTestCase(TestAssetLoaderIntegration))
    suite.addTests(loader.loadTestsFromTestCase(TestAssetValidation))
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
