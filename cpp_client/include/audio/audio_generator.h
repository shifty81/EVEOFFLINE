#ifndef AUDIO_GENERATOR_H
#define AUDIO_GENERATOR_H

#ifdef USE_OPENAL

#include <string>
#include <vector>
#include <cmath>

namespace EVE {

/**
 * Simple audio generator for creating placeholder WAV files
 * Generates sine wave tones for testing
 */
class AudioGenerator {
public:
    // Generate a simple tone and save as WAV
    static bool generate_tone(const std::string& filepath,
                             float frequency_hz,
                             float duration_sec,
                             int sample_rate = 44100,
                             float amplitude = 0.5f);
    
    // Generate a more complex sound (combination of tones)
    static bool generate_multi_tone(const std::string& filepath,
                                   const std::vector<float>& frequencies,
                                   const std::vector<float>& amplitudes,
                                   float duration_sec,
                                   int sample_rate = 44100);
    
    // Generate explosion-like sound (noise burst with decay)
    static bool generate_explosion(const std::string& filepath,
                                  float duration_sec,
                                  int sample_rate = 44100);
    
    // Generate laser-like sound (frequency sweep)
    static bool generate_laser(const std::string& filepath,
                              float start_freq,
                              float end_freq,
                              float duration_sec,
                              int sample_rate = 44100);
    
    // Generate engine-like sound (low frequency rumble)
    static bool generate_engine(const std::string& filepath,
                               float base_freq,
                               float duration_sec,
                               int sample_rate = 44100);

    /**
     * Generate warp tunnel ambient sound (cinematic warp hum/drone).
     * Creates a deep, meditative drone with subtle modulation for long warps.
     * Features:
     *   - Low-frequency bass foundation (40-80 Hz)
     *   - Harmonic overtones for richness
     *   - Slow amplitude modulation (breathing effect)
     *   - Optional shimmer layer for high frequencies
     * @param filepath Output WAV file path
     * @param duration_sec Duration (typically 30-60 seconds for looping)
     * @param mass_factor 0.0 = frigate (higher pitch), 1.0 = capital (deeper bass)
     * @param sample_rate Audio sample rate
     * @return true if generation succeeded
     */
    static bool generate_warp_drone(const std::string& filepath,
                                    float duration_sec,
                                    float mass_factor = 0.5f,
                                    int sample_rate = 44100);

    /**
     * Generate warp entry sound (acceleration into warp).
     * Rising pitch with energy buildup.
     */
    static bool generate_warp_entry(const std::string& filepath,
                                    float duration_sec = 2.0f,
                                    int sample_rate = 44100);

    /**
     * Generate warp exit sound (deceleration from warp).
     * Falling pitch with energy dissipation.
     */
    static bool generate_warp_exit(const std::string& filepath,
                                   float duration_sec = 1.5f,
                                   int sample_rate = 44100);

private:
    // Write WAV file header
    static bool write_wav_header(std::ofstream& file,
                                int sample_rate,
                                int num_samples,
                                int bits_per_sample = 16,
                                int num_channels = 1);
    
    // Convert float sample to int16
    static int16_t float_to_int16(float sample);
};

} // namespace EVE

#endif // USE_OPENAL

#endif // AUDIO_GENERATOR_H
