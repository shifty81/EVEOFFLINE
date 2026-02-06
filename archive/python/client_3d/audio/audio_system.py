"""
Audio System for 3D Client
Manages sound effects and music playback with 3D positioning
"""

from panda3d.core import AudioManager, AudioSound, Vec3
from typing import Optional, Dict
import os
import time
import threading


class AudioSystem:
    """
    Manages audio playback for the 3D client
    Handles sound effects, music, and 3D audio positioning
    """
    
    # Audio categories
    CATEGORY_SFX = "sfx"
    CATEGORY_MUSIC = "music"
    CATEGORY_UI = "ui"
    CATEGORY_VOICE = "voice"
    
    def __init__(self, loader, audio_dir: str = "client_3d/assets/audio"):
        """
        Initialize audio system
        
        Args:
            loader: Panda3D loader instance
            audio_dir: Base directory for audio files
        """
        self.loader = loader
        self.audio_dir = audio_dir
        
        # Audio managers
        try:
            self.audio_mgr = AudioManager.createAudioManager()
            self.music_mgr = AudioManager.createAudioManager()
        except:
            # Fallback if audio not available
            self.audio_mgr = None
            self.music_mgr = None
            print("[AudioSystem] Warning: Audio manager not available")
        
        # Sound caches
        self.sounds: Dict[str, AudioSound] = {}
        self.music: Dict[str, AudioSound] = {}
        
        # Volume settings
        self.master_volume = 1.0
        self.sfx_volume = 0.8
        self.music_volume = 0.6
        self.ui_volume = 0.7
        
        # Current playing music
        self.current_music: Optional[AudioSound] = None
        
        # Fade state
        self.fade_thread: Optional[threading.Thread] = None
        self.fade_stop_flag = threading.Event()
        
        # Create audio directory
        self._ensure_directories()
        
        print(f"[AudioSystem] Initialized with audio directory: {audio_dir}")
    
    def _ensure_directories(self):
        """Create necessary audio directories"""
        dirs = [
            self.audio_dir,
            os.path.join(self.audio_dir, "sfx"),
            os.path.join(self.audio_dir, "music"),
            os.path.join(self.audio_dir, "ui"),
            os.path.join(self.audio_dir, "weapons"),
            os.path.join(self.audio_dir, "engines"),
            os.path.join(self.audio_dir, "explosions"),
        ]
        for dir_path in dirs:
            os.makedirs(dir_path, exist_ok=True)
    
    def is_available(self) -> bool:
        """Check if audio system is available"""
        return self.audio_mgr is not None
    
    def load_sound(self, filename: str, category: str = CATEGORY_SFX, cache: bool = True) -> Optional[AudioSound]:
        """
        Load a sound file
        
        Args:
            filename: Sound filename (relative to audio_dir)
            category: Audio category
            cache: Whether to cache the sound
        
        Returns:
            AudioSound object or None
        """
        if not self.is_available():
            return None
        
        # Check cache
        cache_key = f"{category}:{filename}"
        if cache and cache_key in self.sounds:
            return self.sounds[cache_key]
        
        # Resolve path
        sound_path = self._resolve_path(filename, category)
        if not sound_path:
            print(f"[AudioSystem] Sound not found: {filename}")
            return None
        
        try:
            # Load sound
            if category == self.CATEGORY_MUSIC:
                sound = self.music_mgr.getSound(sound_path)
            else:
                sound = self.audio_mgr.getSound(sound_path)
            
            if sound:
                # Cache if requested
                if cache:
                    self.sounds[cache_key] = sound
                
                print(f"[AudioSystem] Loaded sound: {filename} ({category})")
                return sound
            else:
                print(f"[AudioSystem] Failed to load sound: {filename}")
                return None
        except Exception as e:
            print(f"[AudioSystem] Error loading sound {filename}: {e}")
            return None
    
    def _resolve_path(self, filename: str, category: str) -> Optional[str]:
        """
        Resolve sound filename to full path
        
        Args:
            filename: Sound filename
            category: Audio category
        
        Returns:
            Full path or None
        """
        # Try different locations
        paths_to_try = [
            os.path.join(self.audio_dir, filename),
            os.path.join(self.audio_dir, category, filename),
            filename  # Try as-is
        ]
        
        for path in paths_to_try:
            if os.path.exists(path):
                return path
        
        return None
    
    def play_sound(self, filename: str, volume: float = 1.0, loop: bool = False, 
                   position: Optional[Vec3] = None) -> Optional[AudioSound]:
        """
        Play a sound effect
        
        Args:
            filename: Sound filename
            volume: Volume (0.0 - 1.0)
            loop: Whether to loop the sound
            position: 3D position for spatial audio
        
        Returns:
            AudioSound object or None
        """
        sound = self.load_sound(filename, self.CATEGORY_SFX)
        if sound:
            sound.setVolume(volume * self.sfx_volume * self.master_volume)
            sound.setLoop(loop)
            
            # Set 3D position if provided
            if position:
                sound.set3dAttributes(position.x, position.y, position.z, 0, 0, 0)
            
            sound.play()
            return sound
        
        return None
    
    def play_weapon_sound(self, weapon_type: str, position: Optional[Vec3] = None) -> Optional[AudioSound]:
        """
        Play weapon fire sound
        
        Args:
            weapon_type: Type of weapon (e.g., "laser", "projectile", "missile")
            position: 3D position
        
        Returns:
            AudioSound object or None
        """
        # Map weapon types to sound files
        weapon_sounds = {
            'laser': 'weapons/laser_fire.wav',
            'projectile': 'weapons/projectile_fire.wav',
            'missile': 'weapons/missile_launch.wav',
            'railgun': 'weapons/railgun_fire.wav',
            'blaster': 'weapons/blaster_fire.wav',
        }
        
        sound_file = weapon_sounds.get(weapon_type.lower(), 'weapons/generic_fire.wav')
        return self.play_sound(sound_file, volume=0.7, position=position)
    
    def play_explosion_sound(self, size: str = "medium", position: Optional[Vec3] = None) -> Optional[AudioSound]:
        """
        Play explosion sound
        
        Args:
            size: Size of explosion ("small", "medium", "large")
            position: 3D position
        
        Returns:
            AudioSound object or None
        """
        explosion_sounds = {
            'small': 'explosions/explosion_small.wav',
            'medium': 'explosions/explosion_medium.wav',
            'large': 'explosions/explosion_large.wav',
        }
        
        sound_file = explosion_sounds.get(size.lower(), 'explosions/explosion_medium.wav')
        volume = {'small': 0.5, 'medium': 0.7, 'large': 0.9}.get(size.lower(), 0.7)
        
        return self.play_sound(sound_file, volume=volume, position=position)
    
    def play_engine_sound(self, engine_type: str = "default", loop: bool = True,
                         position: Optional[Vec3] = None) -> Optional[AudioSound]:
        """
        Play ship engine sound
        
        Args:
            engine_type: Type of engine
            loop: Whether to loop the sound
            position: 3D position
        
        Returns:
            AudioSound object or None
        """
        sound_file = f"engines/engine_{engine_type}.wav"
        return self.play_sound(sound_file, volume=0.4, loop=loop, position=position)
    
    def play_ui_sound(self, sound_name: str) -> Optional[AudioSound]:
        """
        Play UI interaction sound
        
        Args:
            sound_name: Name of UI sound (e.g., "click", "hover", "error")
        
        Returns:
            AudioSound object or None
        """
        sound_file = f"ui/{sound_name}.wav"
        sound = self.load_sound(sound_file, self.CATEGORY_UI)
        if sound:
            sound.setVolume(self.ui_volume * self.master_volume)
            sound.play()
            return sound
        return None
    
    def play_music(self, filename: str, loop: bool = True, fade_in: float = 1.0):
        """
        Play background music
        
        Args:
            filename: Music filename
            loop: Whether to loop the music
            fade_in: Fade in duration in seconds
        """
        # Stop current music
        self.stop_music(fade_out=1.0)
        
        # Load and play new music
        music = self.load_sound(filename, self.CATEGORY_MUSIC)
        if music:
            music.setVolume(self.music_volume * self.master_volume)
            music.setLoop(loop)
            music.play()
            
            self.current_music = music
            print(f"[AudioSystem] Playing music: {filename}")
    
    def stop_music(self, fade_out: float = 0.0):
        """
        Stop current music
        
        Args:
            fade_out: Fade out duration in seconds
        """
        if self.current_music and self.current_music.status() == AudioSound.PLAYING:
            if fade_out > 0:
                # Cancel any ongoing fade
                self._cancel_fade()
                
                # Start fade out in separate thread
                self.fade_stop_flag.clear()
                self.fade_thread = threading.Thread(
                    target=self._fade_out_music,
                    args=(self.current_music, fade_out),
                    daemon=True
                )
                self.fade_thread.start()
            else:
                self.current_music.stop()
            
            print("[AudioSystem] Stopped music")
    
    def _fade_out_music(self, music: AudioSound, duration: float):
        """
        Fade out music over specified duration
        
        Args:
            music: Music to fade out
            duration: Fade duration in seconds
        """
        if not music or music.status() != AudioSound.PLAYING:
            return
        
        # Get initial volume
        initial_volume = music.getVolume()
        
        # Fade steps (60 fps for smooth fade)
        steps = int(duration * 60)
        step_duration = duration / steps
        
        for i in range(steps):
            if self.fade_stop_flag.is_set():
                # Fade was cancelled
                return
            
            if music.status() != AudioSound.PLAYING:
                # Music stopped playing
                return
            
            # Calculate new volume (linear fade)
            progress = (i + 1) / steps
            new_volume = initial_volume * (1.0 - progress)
            music.setVolume(new_volume)
            
            # Sleep for step duration
            time.sleep(step_duration)
        
        # Stop music after fade completes
        if music.status() == AudioSound.PLAYING:
            music.stop()
    
    def _cancel_fade(self):
        """Cancel any ongoing fade operation"""
        if self.fade_thread and self.fade_thread.is_alive():
            self.fade_stop_flag.set()
            # Give thread a moment to finish
            self.fade_thread.join(timeout=0.1)
    
    def set_master_volume(self, volume: float):
        """Set master volume (0.0 - 1.0)"""
        self.master_volume = max(0.0, min(1.0, volume))
        print(f"[AudioSystem] Master volume: {self.master_volume}")
    
    def set_sfx_volume(self, volume: float):
        """Set sound effects volume (0.0 - 1.0)"""
        self.sfx_volume = max(0.0, min(1.0, volume))
        print(f"[AudioSystem] SFX volume: {self.sfx_volume}")
    
    def set_music_volume(self, volume: float):
        """Set music volume (0.0 - 1.0)"""
        self.music_volume = max(0.0, min(1.0, volume))
        if self.current_music:
            self.current_music.setVolume(self.music_volume * self.master_volume)
        print(f"[AudioSystem] Music volume: {self.music_volume}")
    
    def set_ui_volume(self, volume: float):
        """Set UI volume (0.0 - 1.0)"""
        self.ui_volume = max(0.0, min(1.0, volume))
        print(f"[AudioSystem] UI volume: {self.ui_volume}")
    
    def update_listener_position(self, position: Vec3, velocity: Vec3 = Vec3(0, 0, 0)):
        """
        Update 3D audio listener position (camera)
        
        Args:
            position: Listener position
            velocity: Listener velocity (for doppler effect)
        """
        if self.is_available() and self.audio_mgr:
            # Update listener position for 3D audio
            self.audio_mgr.audio3dSetListenerAttributes(
                position.x, position.y, position.z,
                velocity.x, velocity.y, velocity.z,
                0, 1, 0,  # Up vector
                0, 0, -1  # Forward vector
            )
    
    def clear_cache(self):
        """Clear all cached sounds"""
        self.sounds.clear()
        self.music.clear()
        print("[AudioSystem] Cache cleared")
    
    def get_cache_info(self) -> Dict[str, int]:
        """Get information about cached audio"""
        return {
            'sounds_cached': len(self.sounds),
            'music_cached': len(self.music)
        }
    
    def list_available_sounds(self) -> Dict[str, list]:
        """
        List all available audio files
        
        Returns:
            Dictionary with categories and their sound files
        """
        sounds = {}
        
        categories = ['sfx', 'music', 'ui', 'weapons', 'engines', 'explosions']
        for category in categories:
            category_dir = os.path.join(self.audio_dir, category)
            if os.path.exists(category_dir):
                sounds[category] = [
                    f for f in os.listdir(category_dir)
                    if f.endswith(('.wav', '.ogg', '.mp3'))
                ]
            else:
                sounds[category] = []
        
        return sounds


class AudioPresets:
    """
    Predefined audio configurations and helper methods
    """
    
    @staticmethod
    def create_combat_ambience(audio_system: AudioSystem):
        """Create combat ambience sound mix"""
        # Play ambient battle sounds
        # This would mix multiple sounds for atmosphere
        pass
    
    @staticmethod
    def create_station_ambience(audio_system: AudioSystem):
        """Create station ambience sound mix"""
        # Play station ambient sounds
        pass
    
    @staticmethod
    def create_warp_effects(audio_system: AudioSystem, position: Vec3):
        """Create warp sound effects"""
        audio_system.play_sound("sfx/warp_enter.wav", volume=0.8, position=position)
