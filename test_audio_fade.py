"""
Test for audio system fade out functionality
"""

import sys
import os

# Mock Panda3D components for testing without full 3D environment
class MockAudioManager:
    @staticmethod
    def createAudioManager():
        return MockAudioManagerInstance()

class MockAudioManagerInstance:
    def getSound(self, path):
        return MockAudioSound()
    
    def audio3dSetListenerAttributes(self, *args):
        pass

class MockAudioSound:
    PLAYING = 1
    STOPPED = 0
    
    def __init__(self):
        self._status = self.STOPPED
        self._volume = 1.0
        self._loop = False
    
    def status(self):
        return self._status
    
    def play(self):
        self._status = self.PLAYING
    
    def stop(self):
        self._status = self.STOPPED
    
    def setVolume(self, volume):
        self._volume = volume
        print(f"  Volume set to: {volume:.3f}")
    
    def getVolume(self):
        return self._volume
    
    def setLoop(self, loop):
        self._loop = loop
    
    def set3dAttributes(self, *args):
        pass

class MockVec3:
    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z

class MockLoader:
    pass

# Mock panda3d.core module
sys.modules['panda3d'] = type(sys)('panda3d')
sys.modules['panda3d.core'] = type(sys)('panda3d.core')
sys.modules['panda3d.core'].AudioManager = MockAudioManager
sys.modules['panda3d.core'].AudioSound = MockAudioSound
sys.modules['panda3d.core'].Vec3 = MockVec3

# Now we can import the audio system
from client_3d.audio.audio_system import AudioSystem

def test_fade_out():
    """Test fade out functionality"""
    print("Testing Audio System Fade Out")
    print("=" * 50)
    
    # Create audio system
    audio_system = AudioSystem(MockLoader(), audio_dir="/tmp/test_audio")
    
    # Create a mock music sound
    music = MockAudioSound()
    music.play()
    music.setVolume(0.8)
    audio_system.current_music = music
    
    print("\n1. Testing fade out over 1 second")
    print("-" * 50)
    initial_volume = music.getVolume()
    print(f"Initial volume: {initial_volume:.3f}")
    
    # Start fade
    audio_system.stop_music(fade_out=1.0)
    
    # Wait for fade to complete
    import time
    time.sleep(1.2)
    
    # Check final state
    if music.status() == MockAudioSound.STOPPED:
        print("✅ Music stopped successfully")
    else:
        print("❌ Music should be stopped")
    
    print("\n2. Testing immediate stop (no fade)")
    print("-" * 50)
    music2 = MockAudioSound()
    music2.play()
    music2.setVolume(0.8)
    audio_system.current_music = music2
    
    print(f"Initial volume: {music2.getVolume():.3f}")
    audio_system.stop_music(fade_out=0.0)
    
    if music2.status() == MockAudioSound.STOPPED:
        print("✅ Music stopped immediately")
    else:
        print("❌ Music should be stopped immediately")
    
    print("\n3. Testing fade cancellation")
    print("-" * 50)
    music3 = MockAudioSound()
    music3.play()
    music3.setVolume(0.8)
    audio_system.current_music = music3
    
    print(f"Initial volume: {music3.getVolume():.3f}")
    print("Starting long fade (2 seconds)...")
    audio_system.stop_music(fade_out=2.0)
    
    # Wait a bit then cancel
    time.sleep(0.3)
    print("Cancelling fade...")
    audio_system._cancel_fade()
    
    # Stop immediately after cancel
    if music3.status() == MockAudioSound.PLAYING:
        music3.stop()
        print("✅ Fade cancelled successfully")
    
    print("\n" + "=" * 50)
    print("All tests completed!")

if __name__ == "__main__":
    test_fade_out()
