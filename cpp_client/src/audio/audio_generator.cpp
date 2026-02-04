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
