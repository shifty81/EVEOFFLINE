#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#ifdef USE_OPENAL

#include <AL/al.h>
#include <AL/alc.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace EVE {

/**
 * Audio buffer resource - loaded audio data
 */
struct AudioBuffer {
    ALuint buffer_id;
    std::string filename;
    float duration;
    
    AudioBuffer() : buffer_id(0), duration(0.0f) {}
    ~AudioBuffer();
};

/**
 * Audio source - actively playing sound
 */
class AudioSource {
public:
    AudioSource();
    ~AudioSource();
    
    void play();
    void pause();
    void stop();
    
    void set_buffer(ALuint buffer_id);
    void set_position(const glm::vec3& pos);
    void set_velocity(const glm::vec3& vel);
    void set_volume(float volume);
    void set_pitch(float pitch);
    void set_looping(bool loop);
    void set_relative(bool relative); // Position relative to listener
    
    bool is_playing() const;
    float get_volume() const;
    ALuint get_source_id() const { return source_id; }
    
private:
    ALuint source_id;
};

/**
 * Main audio manager - handles OpenAL initialization, resource management, and playback
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    // Initialization
    bool initialize();
    void shutdown();
    bool is_initialized() const { return initialized; }
    
    // Audio loading
    bool load_sound(const std::string& name, const std::string& filepath);
    bool load_wav(const std::string& name, const std::string& filepath);
    void unload_sound(const std::string& name);
    void unload_all_sounds();
    
    // Sound playback
    std::shared_ptr<AudioSource> play_sound(const std::string& name, 
                                           float volume = 1.0f,
                                           bool loop = false,
                                           const glm::vec3& position = glm::vec3(0.0f));
    
    std::shared_ptr<AudioSource> play_weapon_sound(const std::string& weapon_type,
                                                   const glm::vec3& position = glm::vec3(0.0f));
    
    std::shared_ptr<AudioSource> play_explosion_sound(const std::string& size = "medium",
                                                      const glm::vec3& position = glm::vec3(0.0f));
    
    std::shared_ptr<AudioSource> play_engine_sound(const std::string& engine_type = "default",
                                                   const glm::vec3& position = glm::vec3(0.0f));
    
    std::shared_ptr<AudioSource> play_ui_sound(const std::string& sound_name);
    
    // Music playback
    void play_music(const std::string& name, float volume = 0.6f, bool loop = true);
    void stop_music();
    void pause_music();
    void resume_music();
    
    // Volume controls
    void set_master_volume(float volume);
    void set_sfx_volume(float volume);
    void set_music_volume(float volume);
    void set_ui_volume(float volume);
    
    float get_master_volume() const { return master_volume; }
    float get_sfx_volume() const { return sfx_volume; }
    float get_music_volume() const { return music_volume; }
    float get_ui_volume() const { return ui_volume; }
    
    // Listener (camera) management
    void set_listener_position(const glm::vec3& position);
    void set_listener_velocity(const glm::vec3& velocity);
    void set_listener_orientation(const glm::vec3& forward, const glm::vec3& up);
    
    // Source pool management
    void update();  // Clean up finished sources
    
    // Utility
    int get_active_sources() const { return active_sources.size(); }
    int get_cached_buffers() const { return audio_buffers.size(); }
    
private:
    // OpenAL context
    ALCdevice* device;
    ALCcontext* context;
    bool initialized;
    
    // Audio resources
    std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> audio_buffers;
    std::vector<std::shared_ptr<AudioSource>> active_sources;
    std::shared_ptr<AudioSource> music_source;
    
    // Volume settings
    float master_volume;
    float sfx_volume;
    float music_volume;
    float ui_volume;
    
    // Helper methods
    std::shared_ptr<AudioSource> create_source();
    void cleanup_finished_sources();
    bool check_al_error(const std::string& operation);
    
    // WAV loading helper
    bool load_wav_file(const std::string& filepath, std::vector<char>& buffer,
                      ALenum& format, ALsizei& frequency);
};

} // namespace EVE

#endif // USE_OPENAL

#endif // AUDIO_MANAGER_H
