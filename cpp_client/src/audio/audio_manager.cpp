#ifdef USE_OPENAL

#include "audio/audio_manager.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>

namespace EVE {

// ============================================================================
// AudioBuffer Implementation
// ============================================================================

AudioBuffer::~AudioBuffer() {
    if (buffer_id != 0) {
        alDeleteBuffers(1, &buffer_id);
    }
}

// ============================================================================
// AudioSource Implementation
// ============================================================================

AudioSource::AudioSource() : source_id(0) {
    alGenSources(1, &source_id);
    
    // Set default properties
    alSourcef(source_id, AL_PITCH, 1.0f);
    alSourcef(source_id, AL_GAIN, 1.0f);
    alSource3f(source_id, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source_id, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcei(source_id, AL_LOOPING, AL_FALSE);
}

AudioSource::~AudioSource() {
    if (source_id != 0) {
        alSourceStop(source_id);
        alDeleteSources(1, &source_id);
    }
}

void AudioSource::play() {
    alSourcePlay(source_id);
}

void AudioSource::pause() {
    alSourcePause(source_id);
}

void AudioSource::stop() {
    alSourceStop(source_id);
}

void AudioSource::set_buffer(ALuint buffer_id) {
    alSourcei(source_id, AL_BUFFER, buffer_id);
}

void AudioSource::set_position(const glm::vec3& pos) {
    alSource3f(source_id, AL_POSITION, pos.x, pos.y, pos.z);
}

void AudioSource::set_velocity(const glm::vec3& vel) {
    alSource3f(source_id, AL_VELOCITY, vel.x, vel.y, vel.z);
}

void AudioSource::set_volume(float volume) {
    alSourcef(source_id, AL_GAIN, std::max(0.0f, std::min(1.0f, volume)));
}

void AudioSource::set_pitch(float pitch) {
    alSourcef(source_id, AL_PITCH, std::max(0.5f, std::min(2.0f, pitch)));
}

void AudioSource::set_looping(bool loop) {
    alSourcei(source_id, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
}

void AudioSource::set_relative(bool relative) {
    alSourcei(source_id, AL_SOURCE_RELATIVE, relative ? AL_TRUE : AL_FALSE);
}

bool AudioSource::is_playing() const {
    ALint state;
    alGetSourcei(source_id, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

float AudioSource::get_volume() const {
    float volume;
    alGetSourcef(source_id, AL_GAIN, &volume);
    return volume;
}

// ============================================================================
// AudioManager Implementation
// ============================================================================

AudioManager::AudioManager()
    : device(nullptr)
    , context(nullptr)
    , initialized(false)
    , master_volume(1.0f)
    , sfx_volume(0.8f)
    , music_volume(0.6f)
    , ui_volume(0.7f)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    if (initialized) {
        return true;
    }
    
    std::cout << "[AudioManager] Initializing OpenAL..." << std::endl;
    
    // Open default audio device
    device = alcOpenDevice(nullptr);
    if (!device) {
        std::cerr << "[AudioManager] Failed to open audio device" << std::endl;
        return false;
    }
    
    // Create audio context
    context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cerr << "[AudioManager] Failed to create audio context" << std::endl;
        alcCloseDevice(device);
        device = nullptr;
        return false;
    }
    
    // Set current context
    if (!alcMakeContextCurrent(context)) {
        std::cerr << "[AudioManager] Failed to make context current" << std::endl;
        alcDestroyContext(context);
        alcCloseDevice(device);
        context = nullptr;
        device = nullptr;
        return false;
    }
    
    // Set listener properties
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    ALfloat orientation[] = { 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f };
    alListenerfv(AL_ORIENTATION, orientation);
    
    // Enable distance attenuation
    alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    
    initialized = true;
    
    std::cout << "[AudioManager] OpenAL initialized successfully" << std::endl;
    std::cout << "[AudioManager] Device: " << alcGetString(device, ALC_DEVICE_SPECIFIER) << std::endl;
    
    return true;
}

void AudioManager::shutdown() {
    if (!initialized) {
        return;
    }
    
    std::cout << "[AudioManager] Shutting down..." << std::endl;
    
    // Stop and cleanup all active sources
    active_sources.clear();
    music_source.reset();
    
    // Delete all audio buffers
    audio_buffers.clear();
    
    // Cleanup OpenAL context
    alcMakeContextCurrent(nullptr);
    if (context) {
        alcDestroyContext(context);
        context = nullptr;
    }
    
    if (device) {
        alcCloseDevice(device);
        device = nullptr;
    }
    
    initialized = false;
    std::cout << "[AudioManager] Shutdown complete" << std::endl;
}

bool AudioManager::load_sound(const std::string& name, const std::string& filepath) {
    if (!initialized) {
        std::cerr << "[AudioManager] Cannot load sound - not initialized" << std::endl;
        return false;
    }
    
    // Check if already loaded
    if (audio_buffers.find(name) != audio_buffers.end()) {
        std::cout << "[AudioManager] Sound already loaded: " << name << std::endl;
        return true;
    }
    
    // Determine file type and load
    if (filepath.size() >= 4 && filepath.substr(filepath.size() - 4) == ".wav") {
        return load_wav(name, filepath);
    }
    
    std::cerr << "[AudioManager] Unsupported audio format: " << filepath << std::endl;
    return false;
}

bool AudioManager::load_wav(const std::string& name, const std::string& filepath) {
    std::vector<char> buffer;
    ALenum format;
    ALsizei frequency;
    
    if (!load_wav_file(filepath, buffer, format, frequency)) {
        return false;
    }
    
    // Create OpenAL buffer
    auto audio_buffer = std::make_shared<AudioBuffer>();
    alGenBuffers(1, &audio_buffer->buffer_id);
    
    if (check_al_error("alGenBuffers")) {
        return false;
    }
    
    // Upload audio data
    alBufferData(audio_buffer->buffer_id, format, buffer.data(), buffer.size(), frequency);
    
    if (check_al_error("alBufferData")) {
        return false;
    }
    
    audio_buffer->filename = filepath;
    audio_buffer->duration = static_cast<float>(buffer.size()) / 
                            (frequency * (format == AL_FORMAT_STEREO16 ? 4 : 2));
    
    audio_buffers[name] = audio_buffer;
    
    std::cout << "[AudioManager] Loaded sound: " << name << " (" << filepath << ")" << std::endl;
    return true;
}

bool AudioManager::load_wav_file(const std::string& filepath, std::vector<char>& buffer,
                                 ALenum& format, ALsizei& frequency) {
    // Open WAV file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioManager] Failed to open WAV file: " << filepath << std::endl;
        return false;
    }
    
    // Read WAV header
    char riff[4];
    file.read(riff, 4);
    if (std::strncmp(riff, "RIFF", 4) != 0) {
        std::cerr << "[AudioManager] Not a valid WAV file (missing RIFF): " << filepath << std::endl;
        return false;
    }
    
    uint32_t file_size;
    file.read(reinterpret_cast<char*>(&file_size), 4);
    
    char wave[4];
    file.read(wave, 4);
    if (std::strncmp(wave, "WAVE", 4) != 0) {
        std::cerr << "[AudioManager] Not a valid WAV file (missing WAVE): " << filepath << std::endl;
        return false;
    }
    
    // Find fmt chunk
    char fmt[4];
    file.read(fmt, 4);
    if (std::strncmp(fmt, "fmt ", 4) != 0) {
        std::cerr << "[AudioManager] Invalid WAV format chunk: " << filepath << std::endl;
        return false;
    }
    
    uint32_t fmt_size;
    file.read(reinterpret_cast<char*>(&fmt_size), 4);
    
    uint16_t audio_format;
    file.read(reinterpret_cast<char*>(&audio_format), 2);
    
    if (audio_format != 1) {  // 1 = PCM
        std::cerr << "[AudioManager] Unsupported WAV format (not PCM): " << filepath << std::endl;
        return false;
    }
    
    uint16_t num_channels;
    file.read(reinterpret_cast<char*>(&num_channels), 2);
    
    uint32_t sample_rate;
    file.read(reinterpret_cast<char*>(&sample_rate), 4);
    
    uint32_t byte_rate;
    file.read(reinterpret_cast<char*>(&byte_rate), 4);
    
    uint16_t block_align;
    file.read(reinterpret_cast<char*>(&block_align), 2);
    
    uint16_t bits_per_sample;
    file.read(reinterpret_cast<char*>(&bits_per_sample), 2);
    
    // Skip any extra format bytes
    if (fmt_size > 16) {
        file.seekg(fmt_size - 16, std::ios::cur);
    }
    
    // Find data chunk
    char data[4];
    file.read(data, 4);
    while (std::strncmp(data, "data", 4) != 0) {
        uint32_t chunk_size;
        file.read(reinterpret_cast<char*>(&chunk_size), 4);
        file.seekg(chunk_size, std::ios::cur);
        file.read(data, 4);
        
        if (file.eof()) {
            std::cerr << "[AudioManager] No data chunk found in WAV file: " << filepath << std::endl;
            return false;
        }
    }
    
    uint32_t data_size;
    file.read(reinterpret_cast<char*>(&data_size), 4);
    
    // Read audio data
    buffer.resize(data_size);
    file.read(buffer.data(), data_size);
    file.close();
    
    // Determine OpenAL format
    if (num_channels == 1) {
        format = (bits_per_sample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    } else if (num_channels == 2) {
        format = (bits_per_sample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
    } else {
        std::cerr << "[AudioManager] Unsupported channel count: " << num_channels << std::endl;
        return false;
    }
    
    frequency = sample_rate;
    
    return true;
}

void AudioManager::unload_sound(const std::string& name) {
    auto it = audio_buffers.find(name);
    if (it != audio_buffers.end()) {
        audio_buffers.erase(it);
        std::cout << "[AudioManager] Unloaded sound: " << name << std::endl;
    }
}

void AudioManager::unload_all_sounds() {
    audio_buffers.clear();
    std::cout << "[AudioManager] Unloaded all sounds" << std::endl;
}

std::shared_ptr<AudioSource> AudioManager::play_sound(const std::string& name, 
                                                      float volume,
                                                      bool loop,
                                                      const glm::vec3& position) {
    if (!initialized) {
        return nullptr;
    }
    
    auto buffer_it = audio_buffers.find(name);
    if (buffer_it == audio_buffers.end()) {
        std::cerr << "[AudioManager] Sound not found: " << name << std::endl;
        return nullptr;
    }
    
    auto source = create_source();
    if (!source) {
        return nullptr;
    }
    
    source->set_buffer(buffer_it->second->buffer_id);
    source->set_volume(volume * sfx_volume * master_volume);
    source->set_looping(loop);
    source->set_position(position);
    source->play();
    
    active_sources.push_back(source);
    
    return source;
}

std::shared_ptr<AudioSource> AudioManager::play_weapon_sound(const std::string& weapon_type,
                                                             const glm::vec3& position) {
    std::string sound_name = "weapon_" + weapon_type;
    return play_sound(sound_name, 0.7f, false, position);
}

std::shared_ptr<AudioSource> AudioManager::play_explosion_sound(const std::string& size,
                                                                const glm::vec3& position) {
    std::string sound_name = "explosion_" + size;
    float volume = 0.7f;
    
    if (size == "small") volume = 0.5f;
    else if (size == "large") volume = 0.9f;
    
    return play_sound(sound_name, volume, false, position);
}

std::shared_ptr<AudioSource> AudioManager::play_engine_sound(const std::string& engine_type,
                                                             const glm::vec3& position) {
    std::string sound_name = "engine_" + engine_type;
    return play_sound(sound_name, 0.4f, true, position);
}

std::shared_ptr<AudioSource> AudioManager::play_ui_sound(const std::string& sound_name) {
    if (!initialized) {
        return nullptr;
    }
    
    std::string full_name = "ui_" + sound_name;
    auto buffer_it = audio_buffers.find(full_name);
    if (buffer_it == audio_buffers.end()) {
        return nullptr;
    }
    
    auto source = create_source();
    if (!source) {
        return nullptr;
    }
    
    source->set_buffer(buffer_it->second->buffer_id);
    source->set_volume(ui_volume * master_volume);
    source->set_looping(false);
    source->set_relative(true);  // UI sounds are relative to listener
    source->play();
    
    active_sources.push_back(source);
    
    return source;
}

void AudioManager::play_music(const std::string& name, float volume, bool loop) {
    if (!initialized) {
        return;
    }
    
    // Stop current music
    stop_music();
    
    auto buffer_it = audio_buffers.find(name);
    if (buffer_it == audio_buffers.end()) {
        std::cerr << "[AudioManager] Music not found: " << name << std::endl;
        return;
    }
    
    music_source = create_source();
    if (!music_source) {
        return;
    }
    
    music_source->set_buffer(buffer_it->second->buffer_id);
    music_source->set_volume(volume * music_volume * master_volume);
    music_source->set_looping(loop);
    music_source->set_relative(true);  // Music doesn't have 3D position
    music_source->play();
    
    std::cout << "[AudioManager] Playing music: " << name << std::endl;
}

void AudioManager::stop_music() {
    if (music_source) {
        music_source->stop();
        music_source.reset();
    }
}

void AudioManager::pause_music() {
    if (music_source) {
        music_source->pause();
    }
}

void AudioManager::resume_music() {
    if (music_source) {
        music_source->play();
    }
}

void AudioManager::set_master_volume(float volume) {
    master_volume = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "[AudioManager] Master volume: " << master_volume << std::endl;
}

void AudioManager::set_sfx_volume(float volume) {
    sfx_volume = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "[AudioManager] SFX volume: " << sfx_volume << std::endl;
}

void AudioManager::set_music_volume(float volume) {
    music_volume = std::max(0.0f, std::min(1.0f, volume));
    if (music_source) {
        music_source->set_volume(music_volume * master_volume);
    }
    std::cout << "[AudioManager] Music volume: " << music_volume << std::endl;
}

void AudioManager::set_ui_volume(float volume) {
    ui_volume = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "[AudioManager] UI volume: " << ui_volume << std::endl;
}

void AudioManager::set_listener_position(const glm::vec3& position) {
    if (initialized) {
        alListener3f(AL_POSITION, position.x, position.y, position.z);
    }
}

void AudioManager::set_listener_velocity(const glm::vec3& velocity) {
    if (initialized) {
        alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
    }
}

void AudioManager::set_listener_orientation(const glm::vec3& forward, const glm::vec3& up) {
    if (initialized) {
        ALfloat orientation[] = { 
            forward.x, forward.y, forward.z,
            up.x, up.y, up.z
        };
        alListenerfv(AL_ORIENTATION, orientation);
    }
}

void AudioManager::update() {
    cleanup_finished_sources();
}

std::shared_ptr<AudioSource> AudioManager::create_source() {
    try {
        auto source = std::make_shared<AudioSource>();
        return source;
    } catch (const std::exception& e) {
        std::cerr << "[AudioManager] Failed to create audio source: " << e.what() << std::endl;
        return nullptr;
    }
}

void AudioManager::cleanup_finished_sources() {
    active_sources.erase(
        std::remove_if(active_sources.begin(), active_sources.end(),
            [](const std::shared_ptr<AudioSource>& source) {
                return !source->is_playing();
            }),
        active_sources.end()
    );
}

bool AudioManager::check_al_error(const std::string& operation) {
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cerr << "[AudioManager] OpenAL error in " << operation << ": ";
        switch (error) {
            case AL_INVALID_NAME:
                std::cerr << "Invalid name" << std::endl;
                break;
            case AL_INVALID_ENUM:
                std::cerr << "Invalid enum" << std::endl;
                break;
            case AL_INVALID_VALUE:
                std::cerr << "Invalid value" << std::endl;
                break;
            case AL_INVALID_OPERATION:
                std::cerr << "Invalid operation" << std::endl;
                break;
            case AL_OUT_OF_MEMORY:
                std::cerr << "Out of memory" << std::endl;
                break;
            default:
                std::cerr << "Unknown error" << std::endl;
                break;
        }
        return true;
    }
    return false;
}

} // namespace EVE

#endif // USE_OPENAL
