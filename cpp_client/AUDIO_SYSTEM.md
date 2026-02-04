# Audio System Documentation

## Overview

The EVE OFFLINE C++ client features a complete 3D audio system built on **OpenAL**. The audio system provides spatial audio, dynamic sound effects, music playback, and comprehensive volume controls.

## Architecture

### Core Components

1. **AudioManager** (`include/audio/audio_manager.h`, `src/audio/audio_manager.cpp`)
   - Main audio management class
   - Handles OpenAL initialization and context management
   - Manages audio resource loading and caching
   - Controls sound playback and 3D positioning
   - Provides volume controls for different audio categories

2. **AudioBuffer**
   - Represents loaded audio data in OpenAL
   - Stores buffer ID, filename, and duration
   - Automatically cleaned up on destruction

3. **AudioSource**
   - Represents an actively playing sound
   - Controls playback (play, pause, stop)
   - Manages 3D position, velocity, volume, pitch
   - Supports looping and relative positioning

4. **AudioGenerator** (`include/audio/audio_generator.h`, `src/audio/audio_generator.cpp`)
   - Utility class for generating placeholder audio files
   - Creates procedural sound effects (tones, explosions, laser sounds, engine sounds)
   - Useful for testing and prototyping

## Features

### 3D Spatial Audio

The audio system supports full 3D spatial audio with:

- **Positional Audio**: Sounds have position in 3D space
- **Distance Attenuation**: Volume decreases with distance from listener
- **Doppler Effect**: Frequency shifts based on relative velocity
- **Listener Orientation**: Camera/player position and orientation affect audio

Example:
```cpp
AudioManager audio;
audio.initialize();

// Play sound at specific 3D position
auto source = audio.play_sound("explosion", 0.7f, false, glm::vec3(10.0f, 0.0f, 5.0f));

// Update listener (camera) position
audio.set_listener_position(camera_position);
audio.set_listener_orientation(camera_forward, camera_up);
```

### Sound Categories

The system supports different categories of audio:

1. **Weapon Sounds**
   - Laser, projectile, missile, railgun, blaster
   - Helper method: `play_weapon_sound()`

2. **Explosion Sounds**
   - Small, medium, large explosions
   - Helper method: `play_explosion_sound()`

3. **Engine Sounds**
   - Ship engine loops with different types
   - Helper method: `play_engine_sound()`

4. **UI Sounds**
   - Button clicks, alerts, confirmations
   - Helper method: `play_ui_sound()`
   - Relative to listener (no 3D positioning)

5. **Music**
   - Background music with loop support
   - Methods: `play_music()`, `stop_music()`, `pause_music()`, `resume_music()`

### Volume Controls

Four-tier volume system:

- **Master Volume**: Global volume control
- **SFX Volume**: Sound effects volume
- **Music Volume**: Background music volume
- **UI Volume**: User interface sounds volume

All volumes range from 0.0 (mute) to 1.0 (full volume).

```cpp
audio.set_master_volume(0.8f);
audio.set_sfx_volume(0.7f);
audio.set_music_volume(0.5f);
audio.set_ui_volume(0.6f);
```

### Resource Management

- **Automatic Caching**: Sounds are cached after first load
- **Source Pooling**: Automatically cleans up finished sources
- **Memory Management**: Smart pointers prevent resource leaks
- **Graceful Degradation**: System works even if OpenAL is not available

```cpp
// Load and cache sound
audio.load_sound("laser", "assets/audio/laser.wav");

// Play multiple times (uses cached buffer)
audio.play_sound("laser", 0.8f);
audio.play_sound("laser", 0.8f);

// Cleanup finished sources
audio.update();
```

## Usage

### Basic Initialization

```cpp
#include "audio/audio_manager.h"

AudioManager audio;

if (!audio.initialize()) {
    std::cerr << "Failed to initialize audio system" << std::endl;
    // Game can continue without audio
}

// Load sounds
audio.load_sound("explosion", "assets/audio/explosion.wav");
audio.load_sound("laser", "assets/audio/laser.wav");

// Play sound
audio.play_sound("explosion", 0.7f);
```

### 3D Spatial Audio

```cpp
// Play sound at specific position
glm::vec3 enemy_pos(10.0f, 2.0f, 5.0f);
auto source = audio.play_weapon_sound("laser", enemy_pos);

// Update source position (e.g., for moving objects)
source->set_position(new_position);
source->set_velocity(velocity);

// Update listener (camera) each frame
audio.set_listener_position(camera->get_position());
audio.set_listener_orientation(camera->get_forward(), camera->get_up());
```

### Combat Scenario

```cpp
// Player fires weapon
audio.play_weapon_sound("laser", player_ship_position);

// Enemy fires back
audio.play_weapon_sound("projectile", enemy_ship_position);

// Hit registered
audio.play_explosion_sound("small", impact_position);

// Ship destroyed
audio.play_explosion_sound("large", ship_position);
```

### UI Interaction

```cpp
// Button click
audio.play_ui_sound("click");

// Hover effect
audio.play_ui_sound("hover");

// Error message
audio.play_ui_sound("error");
```

### Background Music

```cpp
// Start background music
audio.play_music("ambient_space", 0.5f, true);  // 50% volume, looping

// Pause during menu
audio.pause_music();

// Resume
audio.resume_music();

// Stop
audio.stop_music();
```

## File Format Support

Currently supports **WAV files only** (PCM format):
- 8-bit or 16-bit samples
- Mono or stereo channels
- Any sample rate (44100 Hz recommended)

WAV format was chosen for:
- Simplicity (no additional codec libraries needed)
- Wide support
- Lossless quality
- Easy to generate procedurally

### Loading Audio Files

```cpp
// Load from file
audio.load_sound("name", "path/to/sound.wav");

// Generate procedurally (for testing)
AudioGenerator::generate_tone("test.wav", 440.0f, 1.0f);  // 440 Hz, 1 second
AudioGenerator::generate_explosion("boom.wav", 0.8f);
AudioGenerator::generate_laser("laser.wav", 800.0f, 200.0f, 0.3f);
```

## Build Configuration

### CMake Integration

OpenAL is detected automatically and is **optional**:

```cmake
find_package(OpenAL QUIET)
if(OpenAL_FOUND)
    message(STATUS "OpenAL found - audio support enabled")
    add_definitions(-DUSE_OPENAL)
    # Link OpenAL library
    target_link_libraries(eve_client ${OPENAL_LIBRARY})
    target_include_directories(eve_client PRIVATE ${OPENAL_INCLUDE_DIR})
else()
    message(STATUS "OpenAL not found - audio support disabled")
endif()
```

### Conditional Compilation

All audio code is wrapped in `#ifdef USE_OPENAL`:

```cpp
#ifdef USE_OPENAL
#include "audio/audio_manager.h"
// Audio code here
#else
// Fallback or stub implementation
#endif
```

This ensures the game compiles and runs even without OpenAL.

## Testing

### Test Program

Run the audio system test:

```bash
cd cpp_client/build
./bin/test_audio_system
```

The test demonstrates:
1. Basic sound playback
2. 3D spatial audio (sounds moving in space)
3. Multiple simultaneous sounds
4. Volume controls
5. Music playback
6. Listener movement (doppler effect)
7. Combat scenario simulation

### Test Output

```
========================================
  EVE OFFLINE - Audio System Test
========================================

=== Generating Test Sounds ===
[AudioGenerator] Generated tone: assets/audio/weapon_laser.wav
[AudioGenerator] Generated explosion: assets/audio/explosion_medium.wav
...

[AudioManager] Initializing OpenAL...
[AudioManager] OpenAL initialized successfully
[AudioManager] Device: OpenAL Soft

=== Loading Audio Files ===
[AudioManager] Loaded sound: weapon_laser (assets/audio/weapon_laser.wav)
...

=== Test 1: Basic Sound Playback ===
Playing laser sound...
Playing explosion sound...
✓ Basic playback test complete!

... (more tests)

========================================
  All Tests Completed Successfully!
========================================
```

## Platform Support

### Linux

Install OpenAL:
```bash
# Ubuntu/Debian
sudo apt-get install libopenal-dev

# Fedora
sudo dnf install openal-soft-devel
```

### macOS

OpenAL is built into macOS (OpenAL.framework).

### Windows

Download and install OpenAL SDK from openal.org, or use OpenAL-Soft.

## Performance Considerations

1. **Sound Pooling**: Finished sources are automatically cleaned up
2. **Buffer Caching**: Audio buffers are loaded once and reused
3. **Update Frequency**: Call `audio.update()` each frame to clean up finished sources
4. **Source Limits**: OpenAL typically supports 256 simultaneous sources
5. **Memory**: WAV files are loaded entirely into memory (suitable for game SFX)

## Future Enhancements

Potential improvements for future versions:

1. **Streaming Audio**: For long music tracks to reduce memory usage
2. **OGG Vorbis Support**: Compressed audio format
3. **Audio Groups**: Manage related sounds together
4. **Reverb/Echo Effects**: EFX extension for environmental audio
5. **Fade In/Out**: Smooth volume transitions
6. **Playlist System**: Queue multiple music tracks
7. **Audio Events**: Callback system for sound completion
8. **Occlusion**: Sound muffling through obstacles

## API Reference

### AudioManager

#### Initialization
```cpp
bool initialize();
void shutdown();
bool is_initialized() const;
```

#### Audio Loading
```cpp
bool load_sound(const std::string& name, const std::string& filepath);
bool load_wav(const std::string& name, const std::string& filepath);
void unload_sound(const std::string& name);
void unload_all_sounds();
```

#### Sound Playback
```cpp
std::shared_ptr<AudioSource> play_sound(
    const std::string& name, 
    float volume = 1.0f,
    bool loop = false,
    const glm::vec3& position = glm::vec3(0.0f)
);

std::shared_ptr<AudioSource> play_weapon_sound(
    const std::string& weapon_type,
    const glm::vec3& position = glm::vec3(0.0f)
);

std::shared_ptr<AudioSource> play_explosion_sound(
    const std::string& size = "medium",
    const glm::vec3& position = glm::vec3(0.0f)
);

std::shared_ptr<AudioSource> play_engine_sound(
    const std::string& engine_type = "default",
    const glm::vec3& position = glm::vec3(0.0f)
);

std::shared_ptr<AudioSource> play_ui_sound(const std::string& sound_name);
```

#### Music Playback
```cpp
void play_music(const std::string& name, float volume = 0.6f, bool loop = true);
void stop_music();
void pause_music();
void resume_music();
```

#### Volume Controls
```cpp
void set_master_volume(float volume);
void set_sfx_volume(float volume);
void set_music_volume(float volume);
void set_ui_volume(float volume);

float get_master_volume() const;
float get_sfx_volume() const;
float get_music_volume() const;
float get_ui_volume() const;
```

#### Listener Management
```cpp
void set_listener_position(const glm::vec3& position);
void set_listener_velocity(const glm::vec3& velocity);
void set_listener_orientation(const glm::vec3& forward, const glm::vec3& up);
```

#### Utility
```cpp
void update();  // Call each frame
int get_active_sources() const;
int get_cached_buffers() const;
```

### AudioSource

```cpp
void play();
void pause();
void stop();

void set_buffer(ALuint buffer_id);
void set_position(const glm::vec3& pos);
void set_velocity(const glm::vec3& vel);
void set_volume(float volume);
void set_pitch(float pitch);
void set_looping(bool loop);
void set_relative(bool relative);

bool is_playing() const;
float get_volume() const;
```

### AudioGenerator

```cpp
static bool generate_tone(
    const std::string& filepath,
    float frequency_hz,
    float duration_sec,
    int sample_rate = 44100,
    float amplitude = 0.5f
);

static bool generate_explosion(
    const std::string& filepath,
    float duration_sec,
    int sample_rate = 44100
);

static bool generate_laser(
    const std::string& filepath,
    float start_freq,
    float end_freq,
    float duration_sec,
    int sample_rate = 44100
);

static bool generate_engine(
    const std::string& filepath,
    float base_freq,
    float duration_sec,
    int sample_rate = 44100
);
```

## Troubleshooting

### "Failed to initialize audio system"

- Ensure OpenAL is installed on your system
- Check if audio device is available and not in use
- Try running with administrator/root privileges

### No sound output

- Check volume settings (master, SFX, music, UI)
- Verify audio files are loaded correctly
- Check if source is actually playing (`source->is_playing()`)
- Ensure listener position is reasonable

### Crackling or distortion

- Check if too many sounds are playing simultaneously
- Reduce master volume
- Verify WAV files are valid PCM format

### Sounds not positioned correctly

- Verify listener position and orientation are updated each frame
- Check source positions are in correct coordinate system
- Ensure distance attenuation model is appropriate

## Conclusion

The audio system provides a complete, production-ready solution for game audio with:
- ✅ 3D spatial audio with doppler effect
- ✅ Multiple sound categories
- ✅ Comprehensive volume controls
- ✅ Resource management and caching
- ✅ Optional dependency (works without OpenAL)
- ✅ Cross-platform support
- ✅ Simple, intuitive API

The system integrates seamlessly with the EVE OFFLINE client and is ready for use in gameplay!
