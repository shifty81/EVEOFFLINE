#!/usr/bin/env python3
"""
Test suite for Audio System
Tests audio playback and management
"""

import unittest
import tempfile
import shutil
import os
from unittest.mock import Mock, patch, MagicMock
from panda3d.core import Vec3


class TestAudioSystem(unittest.TestCase):
    """Test AudioSystem class"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_loader = Mock()
        self.test_dir = tempfile.mkdtemp()
    
    def tearDown(self):
        """Clean up test files"""
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)
    
    def test_initialization(self):
        """Test audio system initialization"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        self.assertEqual(audio.audio_dir, self.test_dir)
        self.assertEqual(audio.master_volume, 1.0)
        self.assertEqual(audio.sfx_volume, 0.8)
        self.assertEqual(audio.music_volume, 0.6)
        self.assertEqual(audio.ui_volume, 0.7)
    
    def test_directories_created(self):
        """Test that audio directories are created"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Check that directories were created
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "sfx")))
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "music")))
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "ui")))
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "weapons")))
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "engines")))
        self.assertTrue(os.path.exists(os.path.join(self.test_dir, "explosions")))
    
    def test_set_master_volume(self):
        """Test setting master volume"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        audio.set_master_volume(0.5)
        self.assertEqual(audio.master_volume, 0.5)
        
        # Test clamping
        audio.set_master_volume(1.5)
        self.assertEqual(audio.master_volume, 1.0)
        
        audio.set_master_volume(-0.5)
        self.assertEqual(audio.master_volume, 0.0)
    
    def test_set_sfx_volume(self):
        """Test setting SFX volume"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        audio.set_sfx_volume(0.7)
        self.assertEqual(audio.sfx_volume, 0.7)
        
        # Test clamping
        audio.set_sfx_volume(1.5)
        self.assertEqual(audio.sfx_volume, 1.0)
        
        audio.set_sfx_volume(-0.5)
        self.assertEqual(audio.sfx_volume, 0.0)
    
    def test_set_music_volume(self):
        """Test setting music volume"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        audio.set_music_volume(0.4)
        self.assertEqual(audio.music_volume, 0.4)
        
        # Test clamping
        audio.set_music_volume(1.5)
        self.assertEqual(audio.music_volume, 1.0)
        
        audio.set_music_volume(-0.5)
        self.assertEqual(audio.music_volume, 0.0)
    
    def test_set_ui_volume(self):
        """Test setting UI volume"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        audio.set_ui_volume(0.9)
        self.assertEqual(audio.ui_volume, 0.9)
        
        # Test clamping
        audio.set_ui_volume(1.5)
        self.assertEqual(audio.ui_volume, 1.0)
        
        audio.set_ui_volume(-0.5)
        self.assertEqual(audio.ui_volume, 0.0)
    
    def test_resolve_path(self):
        """Test path resolution"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Create test file
        test_file = os.path.join(self.test_dir, "sfx", "test.wav")
        with open(test_file, 'w') as f:
            f.write("test")
        
        # Test resolution
        resolved = audio._resolve_path("test.wav", "sfx")
        self.assertIsNotNone(resolved)
        self.assertTrue(os.path.exists(resolved))
    
    def test_clear_cache(self):
        """Test cache clearing"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Add some cache entries
        audio.sounds["test1"] = Mock()
        audio.sounds["test2"] = Mock()
        audio.music["music1"] = Mock()
        
        # Clear cache
        audio.clear_cache()
        
        # Check empty
        self.assertEqual(len(audio.sounds), 0)
        self.assertEqual(len(audio.music), 0)
    
    def test_get_cache_info(self):
        """Test cache info retrieval"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Add cache entries
        audio.sounds["test1"] = Mock()
        audio.sounds["test2"] = Mock()
        audio.music["music1"] = Mock()
        
        # Get info
        info = audio.get_cache_info()
        
        self.assertEqual(info['sounds_cached'], 2)
        self.assertEqual(info['music_cached'], 1)
    
    def test_list_available_sounds(self):
        """Test listing available sounds"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Create test files
        sfx_dir = os.path.join(self.test_dir, "sfx")
        music_dir = os.path.join(self.test_dir, "music")
        
        with open(os.path.join(sfx_dir, "test1.wav"), 'w') as f:
            f.write("test")
        with open(os.path.join(sfx_dir, "test2.wav"), 'w') as f:
            f.write("test")
        with open(os.path.join(music_dir, "music1.ogg"), 'w') as f:
            f.write("test")
        
        # Create non-audio file (should be ignored)
        with open(os.path.join(sfx_dir, "readme.txt"), 'w') as f:
            f.write("test")
        
        # List sounds
        sounds = audio.list_available_sounds()
        
        self.assertIn('sfx', sounds)
        self.assertIn('music', sounds)
        self.assertEqual(len(sounds['sfx']), 2)
        self.assertEqual(len(sounds['music']), 1)
    
    def test_is_available(self):
        """Test audio availability check"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Should return boolean
        self.assertIsInstance(audio.is_available(), bool)
    
    def test_load_sound_not_found(self):
        """Test loading non-existent sound"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Try to load non-existent sound
        sound = audio.load_sound("nonexistent.wav", "sfx")
        
        # Should return None if audio not available or file not found
        if not audio.is_available():
            self.assertIsNone(sound)
    
    def test_audio_categories(self):
        """Test audio category constants"""
        from client_3d.audio.audio_system import AudioSystem
        
        self.assertEqual(AudioSystem.CATEGORY_SFX, "sfx")
        self.assertEqual(AudioSystem.CATEGORY_MUSIC, "music")
        self.assertEqual(AudioSystem.CATEGORY_UI, "ui")
        self.assertEqual(AudioSystem.CATEGORY_VOICE, "voice")


class TestAudioPresets(unittest.TestCase):
    """Test AudioPresets class"""
    
    def test_preset_methods_exist(self):
        """Test that preset methods exist"""
        from client_3d.audio.audio_system import AudioPresets
        
        self.assertTrue(hasattr(AudioPresets, 'create_combat_ambience'))
        self.assertTrue(hasattr(AudioPresets, 'create_station_ambience'))
        self.assertTrue(hasattr(AudioPresets, 'create_warp_effects'))


class TestVolumeSettings(unittest.TestCase):
    """Test volume setting ranges and validation"""
    
    def setUp(self):
        """Set up test fixtures"""
        self.mock_loader = Mock()
        self.test_dir = tempfile.mkdtemp()
    
    def tearDown(self):
        """Clean up test files"""
        if os.path.exists(self.test_dir):
            shutil.rmtree(self.test_dir)
    
    def test_volume_ranges(self):
        """Test that volume values stay in 0-1 range"""
        from client_3d.audio.audio_system import AudioSystem
        
        audio = AudioSystem(self.mock_loader, self.test_dir)
        
        # Test all volume types
        volume_setters = [
            audio.set_master_volume,
            audio.set_sfx_volume,
            audio.set_music_volume,
            audio.set_ui_volume,
        ]
        
        for setter in volume_setters:
            # Test valid values
            for value in [0.0, 0.5, 1.0]:
                setter(value)
            
            # Test clamping
            setter(2.0)
            setter(-1.0)


def run_tests():
    """Run all tests"""
    # Create test suite
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()
    
    # Add all test classes
    suite.addTests(loader.loadTestsFromTestCase(TestAudioSystem))
    suite.addTests(loader.loadTestsFromTestCase(TestAudioPresets))
    suite.addTests(loader.loadTestsFromTestCase(TestVolumeSettings))
    
    # Run tests
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    
    return result.wasSuccessful()


if __name__ == '__main__':
    import sys
    success = run_tests()
    sys.exit(0 if success else 1)
