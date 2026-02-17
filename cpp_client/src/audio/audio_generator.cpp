#ifdef USE_OPENAL

#include "audio/audio_generator.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <random>

namespace EVE {

bool AudioGenerator::generate_tone(const std::string& filepath,
                                   float frequency_hz,
                                   float duration_sec,
                                   int sample_rate,
                                   float amplitude) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        float value = amplitude * std::sin(two_pi * frequency_hz * t);
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated tone: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_multi_tone(const std::string& filepath,
                                        const std::vector<float>& frequencies,
                                        const std::vector<float>& amplitudes,
                                        float duration_sec,
                                        int sample_rate) {
    if (frequencies.empty() || frequencies.size() != amplitudes.size()) {
        std::cerr << "[AudioGenerator] Invalid frequency/amplitude arrays" << std::endl;
        return false;
    }
    
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples, 0);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        float value = 0.0f;
        
        for (size_t j = 0; j < frequencies.size(); ++j) {
            value += amplitudes[j] * std::sin(two_pi * frequencies[j] * t);
        }
        
        // Normalize by number of tones
        value /= frequencies.size();
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated multi-tone: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_explosion(const std::string& filepath,
                                       float duration_sec,
                                       int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;
        
        // Exponential decay envelope
        float envelope = std::exp(-5.0f * t);
        
        // White noise
        float noise = dis(gen);
        
        // Add some low frequency rumble
        float rumble = 0.3f * std::sin(2.0f * 3.14159265359f * 80.0f * t);
        
        float value = envelope * (0.7f * noise + 0.3f * rumble);
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated explosion: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_laser(const std::string& filepath,
                                   float start_freq,
                                   float end_freq,
                                   float duration_sec,
                                   int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;
        
        // Linear frequency sweep
        float freq = start_freq + (end_freq - start_freq) * t;
        
        // Exponential decay envelope
        float envelope = std::exp(-3.0f * t);
        
        float value = envelope * std::sin(two_pi * freq * static_cast<float>(i) / sample_rate);
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated laser: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_engine(const std::string& filepath,
                                    float base_freq,
                                    float duration_sec,
                                    int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-0.1f, 0.1f);
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        
        // Base tone with harmonics
        float value = 0.4f * std::sin(two_pi * base_freq * t);
        value += 0.3f * std::sin(two_pi * base_freq * 2.0f * t);
        value += 0.2f * std::sin(two_pi * base_freq * 3.0f * t);
        
        // Add slight randomness for realism
        value += 0.1f * dis(gen);
        
        // Slight amplitude modulation
        value *= (1.0f + 0.1f * std::sin(two_pi * 5.0f * t));
        
        samples[i] = float_to_int16(value * 0.5f);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated engine: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_warp_drone(const std::string& filepath,
                                         float duration_sec,
                                         float mass_factor,
                                         int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    // Mass factor affects base frequency: frigates higher pitch, capitals deeper
    // Frigate (mass=0): base ~60Hz, Capital (mass=1): base ~35Hz
    float base_freq = 60.0f - 25.0f * mass_factor;
    
    // Breathing modulation rate (slower for heavier ships = more meditative)
    float breath_rate = 0.08f - 0.03f * mass_factor;  // 0.08 Hz (frigate) to 0.05 Hz (capital)
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> shimmer_dis(-0.02f, 0.02f);
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / sample_rate;
        
        // Slow breathing amplitude envelope (subtle pulsing)
        float breath = 0.85f + 0.15f * std::sin(two_pi * breath_rate * t);
        
        // Base drone with harmonics
        float drone = 0.35f * std::sin(two_pi * base_freq * t);           // Fundamental
        drone += 0.25f * std::sin(two_pi * base_freq * 2.0f * t);         // 1st harmonic
        drone += 0.15f * std::sin(two_pi * base_freq * 3.0f * t);         // 2nd harmonic
        drone += 0.10f * std::sin(two_pi * base_freq * 5.0f * t);         // 4th harmonic (skip 4th for organ-like)
        
        // Sub-bass rumble (very low, felt more than heard)
        float sub_bass = 0.20f * std::sin(two_pi * (base_freq * 0.5f) * t);
        
        // High-frequency shimmer (subtle sparkle, like distant stars)
        float shimmer = 0.03f * std::sin(two_pi * 440.0f * t) * 
                        (0.5f + 0.5f * std::sin(two_pi * 0.2f * t));
        shimmer += shimmer_dis(gen);  // Tiny random variation
        
        // Combine all layers
        float value = breath * (drone + sub_bass) + shimmer;
        
        // Apply soft limiter
        value = std::tanh(value * 1.2f) * 0.8f;
        
        samples[i] = float_to_int16(value * 0.6f);  // Master volume
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated warp drone: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_warp_entry(const std::string& filepath,
                                         float duration_sec,
                                         int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;  // Normalized 0-1
        float time_sec = static_cast<float>(i) / sample_rate;
        
        // Rising amplitude envelope (builds energy)
        float envelope = std::pow(t, 0.7f);
        
        // Rising frequency sweep (40Hz -> 200Hz)
        float freq = 40.0f + 160.0f * t * t;  // Quadratic rise
        
        // Main tone with harmonics
        float tone = 0.5f * std::sin(two_pi * freq * time_sec);
        tone += 0.3f * std::sin(two_pi * freq * 2.0f * time_sec);
        tone += 0.15f * std::sin(two_pi * freq * 3.0f * time_sec);
        
        // High-frequency shimmer that increases
        float shimmer = 0.1f * t * std::sin(two_pi * 800.0f * time_sec) *
                        std::sin(two_pi * 4.0f * time_sec);
        
        // Woosh/rush sound (filtered noise rising)
        float woosh = 0.15f * t * std::sin(two_pi * 150.0f * time_sec * (1.0f + t));
        
        float value = envelope * (tone + shimmer + woosh);
        
        // Soft clip
        value = std::tanh(value * 1.5f) * 0.85f;
        
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated warp entry: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::generate_warp_exit(const std::string& filepath,
                                        float duration_sec,
                                        int sample_rate) {
    int num_samples = static_cast<int>(sample_rate * duration_sec);
    std::vector<int16_t> samples(num_samples);
    
    const float two_pi = 2.0f * 3.14159265359f;
    
    for (int i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;  // Normalized 0-1
        float time_sec = static_cast<float>(i) / sample_rate;
        
        // Falling amplitude envelope (energy dissipating)
        float envelope = 1.0f - std::pow(t, 0.5f);
        
        // Falling frequency sweep (200Hz -> 40Hz)
        float freq = 200.0f - 160.0f * t;
        
        // Main tone with harmonics (fading)
        float tone = 0.5f * std::sin(two_pi * freq * time_sec);
        tone += 0.3f * (1.0f - t) * std::sin(two_pi * freq * 2.0f * time_sec);
        tone += 0.15f * (1.0f - t) * std::sin(two_pi * freq * 3.0f * time_sec);
        
        // Arrival "bloom" (brief brightness at start)
        float bloom = 0.3f * std::exp(-10.0f * t) * std::sin(two_pi * 300.0f * time_sec);
        
        // Spatial reverb tail (subtle echo decay)
        float reverb = 0.1f * std::exp(-5.0f * t) * 
                       std::sin(two_pi * 80.0f * time_sec * (1.0f + 0.5f * t));
        
        float value = envelope * (tone + reverb) + bloom;
        
        // Soft clip
        value = std::tanh(value * 1.3f) * 0.8f;
        
        samples[i] = float_to_int16(value);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[AudioGenerator] Failed to create file: " << filepath << std::endl;
        return false;
    }
    
    write_wav_header(file, sample_rate, num_samples);
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(int16_t));
    file.close();
    
    std::cout << "[AudioGenerator] Generated warp exit: " << filepath << std::endl;
    return true;
}

bool AudioGenerator::write_wav_header(std::ofstream& file,
                                     int sample_rate,
                                     int num_samples,
                                     int bits_per_sample,
                                     int num_channels) {
    int byte_rate = sample_rate * num_channels * bits_per_sample / 8;
    int block_align = num_channels * bits_per_sample / 8;
    int data_size = num_samples * num_channels * bits_per_sample / 8;
    int file_size = 36 + data_size;
    
    // RIFF header
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&file_size), 4);
    file.write("WAVE", 4);
    
    // fmt chunk
    file.write("fmt ", 4);
    int fmt_size = 16;
    file.write(reinterpret_cast<const char*>(&fmt_size), 4);
    
    int16_t audio_format = 1;  // PCM
    file.write(reinterpret_cast<const char*>(&audio_format), 2);
    
    int16_t channels = num_channels;
    file.write(reinterpret_cast<const char*>(&channels), 2);
    
    file.write(reinterpret_cast<const char*>(&sample_rate), 4);
    file.write(reinterpret_cast<const char*>(&byte_rate), 4);
    
    int16_t block_align_16 = block_align;
    file.write(reinterpret_cast<const char*>(&block_align_16), 2);
    
    int16_t bits_per_sample_16 = bits_per_sample;
    file.write(reinterpret_cast<const char*>(&bits_per_sample_16), 2);
    
    // data chunk
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&data_size), 4);
    
    return true;
}

int16_t AudioGenerator::float_to_int16(float sample) {
    sample = std::max(-1.0f, std::min(1.0f, sample));
    return static_cast<int16_t>(sample * 32767.0f);
}

} // namespace EVE

#endif // USE_OPENAL
