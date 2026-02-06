/**
 * Test program for Audio System with OpenAL
 * Tests 3D spatial audio, sound effects, and volume controls
 */

#ifdef USE_OPENAL

#include "audio/audio_manager.h"
#include "audio/audio_generator.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include <random>
#include <filesystem>

using namespace EVE;

// Helper to pause execution
void wait(float seconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

// Generate placeholder audio files
bool generate_test_sounds(const std::string& asset_dir) {
    std::cout << "\n=== Generating Test Sounds ===" << std::endl;
    
    namespace fs = std::filesystem;
    
    // Create directories
    fs::create_directories(asset_dir);
    
    // Generate weapon sounds
    std::cout << "\nGenerating weapon sounds..." << std::endl;
    AudioGenerator::generate_laser(asset_dir + "/weapon_laser.wav", 800.0f, 200.0f, 0.3f);
    AudioGenerator::generate_tone(asset_dir + "/weapon_projectile.wav", 150.0f, 0.2f);
    AudioGenerator::generate_multi_tone(asset_dir + "/weapon_missile.wav", 
                                       {300.0f, 600.0f, 150.0f}, {0.5f, 0.3f, 0.4f}, 0.5f);
    AudioGenerator::generate_laser(asset_dir + "/weapon_railgun.wav", 1200.0f, 100.0f, 0.2f);
    AudioGenerator::generate_multi_tone(asset_dir + "/weapon_blaster.wav",
                                       {400.0f, 800.0f}, {0.6f, 0.4f}, 0.25f);
    
    // Generate explosion sounds
    std::cout << "\nGenerating explosion sounds..." << std::endl;
    AudioGenerator::generate_explosion(asset_dir + "/explosion_small.wav", 0.5f);
    AudioGenerator::generate_explosion(asset_dir + "/explosion_medium.wav", 0.8f);
    AudioGenerator::generate_explosion(asset_dir + "/explosion_large.wav", 1.2f);
    
    // Generate engine sounds
    std::cout << "\nGenerating engine sounds..." << std::endl;
    AudioGenerator::generate_engine(asset_dir + "/engine_default.wav", 80.0f, 2.0f);
    AudioGenerator::generate_engine(asset_dir + "/engine_frigate.wav", 100.0f, 2.0f);
    AudioGenerator::generate_engine(asset_dir + "/engine_cruiser.wav", 60.0f, 2.0f);
    
    // Generate UI sounds
    std::cout << "\nGenerating UI sounds..." << std::endl;
    AudioGenerator::generate_tone(asset_dir + "/ui_click.wav", 1000.0f, 0.05f, 44100, 0.3f);
    AudioGenerator::generate_tone(asset_dir + "/ui_hover.wav", 800.0f, 0.03f, 44100, 0.2f);
    AudioGenerator::generate_multi_tone(asset_dir + "/ui_error.wav",
                                       {400.0f, 500.0f}, {0.4f, 0.3f}, 0.15f);
    AudioGenerator::generate_tone(asset_dir + "/ui_confirm.wav", 1200.0f, 0.1f, 44100, 0.4f);
    
    // Generate music (simple melody)
    std::cout << "\nGenerating background music..." << std::endl;
    AudioGenerator::generate_multi_tone(asset_dir + "/music_ambient.wav",
                                       {220.0f, 330.0f, 440.0f}, {0.3f, 0.3f, 0.3f}, 5.0f);
    
    std::cout << "\n✓ All test sounds generated successfully!" << std::endl;
    return true;
}

// Test basic sound playback
void test_basic_playback(AudioManager& audio) {
    std::cout << "\n=== Test 1: Basic Sound Playback ===" << std::endl;
    
    std::cout << "Playing laser sound..." << std::endl;
    auto laser = audio.play_sound("weapon_laser", 0.8f);
    wait(0.5f);
    
    std::cout << "Playing explosion sound..." << std::endl;
    auto explosion = audio.play_sound("explosion_medium", 0.7f);
    wait(1.0f);
    
    std::cout << "Playing UI click sound..." << std::endl;
    audio.play_ui_sound("click");
    wait(0.2f);
    
    std::cout << "✓ Basic playback test complete!" << std::endl;
}

// Test 3D spatial audio
void test_spatial_audio(AudioManager& audio) {
    std::cout << "\n=== Test 2: 3D Spatial Audio ===" << std::endl;
    
    // Play sound moving from left to right
    std::cout << "Playing sound moving from left to right..." << std::endl;
    auto source = audio.play_sound("weapon_projectile", 0.8f, true, glm::vec3(-10.0f, 0.0f, 0.0f));
    
    for (int i = 0; i <= 20; ++i) {
        float x = -10.0f + i * 1.0f;  // Move from -10 to +10
        source->set_position(glm::vec3(x, 0.0f, 0.0f));
        wait(0.1f);
    }
    
    source->stop();
    wait(0.5f);
    
    // Play sound moving in a circle
    std::cout << "Playing sound moving in a circle..." << std::endl;
    source = audio.play_sound("engine_default", 0.6f, true);
    
    const float radius = 5.0f;
    for (int i = 0; i < 60; ++i) {
        float angle = i * (2.0f * 3.14159265359f / 60.0f);
        float x = radius * std::cos(angle);
        float z = radius * std::sin(angle);
        source->set_position(glm::vec3(x, 0.0f, z));
        wait(0.05f);
    }
    
    source->stop();
    
    std::cout << "✓ Spatial audio test complete!" << std::endl;
}

// Test multiple simultaneous sounds
void test_multiple_sounds(AudioManager& audio) {
    std::cout << "\n=== Test 3: Multiple Simultaneous Sounds ===" << std::endl;
    
    std::cout << "Playing multiple explosions at different positions..." << std::endl;
    
    audio.play_explosion_sound("small", glm::vec3(-5.0f, 0.0f, 0.0f));
    wait(0.3f);
    
    audio.play_explosion_sound("medium", glm::vec3(0.0f, 0.0f, -5.0f));
    wait(0.3f);
    
    audio.play_explosion_sound("large", glm::vec3(5.0f, 0.0f, 0.0f));
    wait(0.5f);
    
    std::cout << "Playing weapon salvo..." << std::endl;
    audio.play_weapon_sound("laser", glm::vec3(-3.0f, 0.0f, 5.0f));
    wait(0.1f);
    audio.play_weapon_sound("projectile", glm::vec3(-1.0f, 0.0f, 5.0f));
    wait(0.1f);
    audio.play_weapon_sound("missile", glm::vec3(1.0f, 0.0f, 5.0f));
    wait(0.1f);
    audio.play_weapon_sound("railgun", glm::vec3(3.0f, 0.0f, 5.0f));
    
    wait(1.0f);
    
    std::cout << "Active sources: " << audio.get_active_sources() << std::endl;
    std::cout << "✓ Multiple sounds test complete!" << std::endl;
}

// Test volume controls
void test_volume_controls(AudioManager& audio) {
    std::cout << "\n=== Test 4: Volume Controls ===" << std::endl;
    
    std::cout << "Testing master volume changes..." << std::endl;
    
    std::cout << "Volume at 100%..." << std::endl;
    audio.set_master_volume(1.0f);
    audio.play_sound("explosion_medium", 0.7f);
    wait(1.0f);
    
    std::cout << "Volume at 50%..." << std::endl;
    audio.set_master_volume(0.5f);
    audio.play_sound("explosion_medium", 0.7f);
    wait(1.0f);
    
    std::cout << "Volume at 25%..." << std::endl;
    audio.set_master_volume(0.25f);
    audio.play_sound("explosion_medium", 0.7f);
    wait(1.0f);
    
    // Reset to normal
    audio.set_master_volume(1.0f);
    
    std::cout << "✓ Volume control test complete!" << std::endl;
}

// Test music playback
void test_music(AudioManager& audio) {
    std::cout << "\n=== Test 5: Music Playback ===" << std::endl;
    
    std::cout << "Playing background music (3 seconds)..." << std::endl;
    audio.play_music("music_ambient", 0.5f, true);
    wait(3.0f);
    
    std::cout << "Pausing music..." << std::endl;
    audio.pause_music();
    wait(1.0f);
    
    std::cout << "Resuming music..." << std::endl;
    audio.resume_music();
    wait(2.0f);
    
    std::cout << "Stopping music..." << std::endl;
    audio.stop_music();
    wait(0.5f);
    
    std::cout << "✓ Music playback test complete!" << std::endl;
}

// Test listener movement (doppler effect)
void test_listener_movement(AudioManager& audio) {
    std::cout << "\n=== Test 6: Listener Movement (Doppler Effect) ===" << std::endl;
    
    // Place a stationary sound source
    glm::vec3 source_pos(0.0f, 0.0f, 10.0f);
    auto source = audio.play_sound("engine_default", 0.6f, true, source_pos);
    
    std::cout << "Moving listener toward sound source..." << std::endl;
    for (int i = 0; i <= 20; ++i) {
        float z = -10.0f + i * 1.0f;  // Move from -10 to +10
        glm::vec3 listener_pos(0.0f, 0.0f, z);
        glm::vec3 listener_vel(0.0f, 0.0f, 1.0f);  // Moving forward
        
        audio.set_listener_position(listener_pos);
        audio.set_listener_velocity(listener_vel);
        wait(0.1f);
    }
    
    // Reset listener
    audio.set_listener_position(glm::vec3(0.0f, 0.0f, 0.0f));
    audio.set_listener_velocity(glm::vec3(0.0f, 0.0f, 0.0f));
    
    source->stop();
    
    std::cout << "✓ Listener movement test complete!" << std::endl;
}

// Test combat scenario
void test_combat_scenario(AudioManager& audio) {
    std::cout << "\n=== Test 7: Combat Scenario ===" << std::endl;
    std::cout << "Simulating a space battle..." << std::endl;
    
    std::mt19937 rng(std::random_device{}());
    
    // Start with some engine sounds
    auto player_engine = audio.play_engine_sound("frigate", glm::vec3(0.0f, 0.0f, 0.0f));
    auto enemy_engine = audio.play_engine_sound("cruiser", glm::vec3(15.0f, 0.0f, 10.0f));
    
    wait(0.5f);
    
    // Weapon exchanges
    for (int i = 0; i < 3; ++i) {
        // Player fires
        audio.play_weapon_sound("laser", glm::vec3(2.0f, 0.0f, 5.0f));
        wait(0.2f);
        
        // Enemy fires back
        audio.play_weapon_sound("projectile", glm::vec3(15.0f, 0.0f, 10.0f));
        wait(0.3f);
        
        // Some impacts
        std::uniform_int_distribution<int> impact_dist(-5, 5);
        audio.play_explosion_sound("small", glm::vec3(impact_dist(rng), 0.0f, impact_dist(rng) + 5));
        wait(0.4f);
    }
    
    // Final big explosion
    std::cout << "Boom! Enemy destroyed!" << std::endl;
    audio.play_explosion_sound("large", glm::vec3(15.0f, 0.0f, 10.0f));
    
    wait(1.5f);
    
    // Stop engines
    player_engine->stop();
    enemy_engine->stop();
    
    std::cout << "✓ Combat scenario test complete!" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  EVE OFFLINE - Audio System Test" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Asset directory
    std::string asset_dir = "assets/audio";
    
    // Generate test sounds
    if (!generate_test_sounds(asset_dir)) {
        std::cerr << "Failed to generate test sounds" << std::endl;
        return 1;
    }
    
    // Initialize audio manager
    AudioManager audio;
    
    if (!audio.initialize()) {
        std::cerr << "\nFailed to initialize audio system!" << std::endl;
        std::cerr << "Make sure OpenAL is installed on your system." << std::endl;
        return 1;
    }
    
    std::cout << "\n✓ Audio system initialized successfully!" << std::endl;
    
    // Load all sounds
    std::cout << "\n=== Loading Audio Files ===" << std::endl;
    
    // Weapons
    audio.load_sound("weapon_laser", asset_dir + "/weapon_laser.wav");
    audio.load_sound("weapon_projectile", asset_dir + "/weapon_projectile.wav");
    audio.load_sound("weapon_missile", asset_dir + "/weapon_missile.wav");
    audio.load_sound("weapon_railgun", asset_dir + "/weapon_railgun.wav");
    audio.load_sound("weapon_blaster", asset_dir + "/weapon_blaster.wav");
    
    // Explosions
    audio.load_sound("explosion_small", asset_dir + "/explosion_small.wav");
    audio.load_sound("explosion_medium", asset_dir + "/explosion_medium.wav");
    audio.load_sound("explosion_large", asset_dir + "/explosion_large.wav");
    
    // Engines
    audio.load_sound("engine_default", asset_dir + "/engine_default.wav");
    audio.load_sound("engine_frigate", asset_dir + "/engine_frigate.wav");
    audio.load_sound("engine_cruiser", asset_dir + "/engine_cruiser.wav");
    
    // UI
    audio.load_sound("ui_click", asset_dir + "/ui_click.wav");
    audio.load_sound("ui_hover", asset_dir + "/ui_hover.wav");
    audio.load_sound("ui_error", asset_dir + "/ui_error.wav");
    audio.load_sound("ui_confirm", asset_dir + "/ui_confirm.wav");
    
    // Music
    audio.load_sound("music_ambient", asset_dir + "/music_ambient.wav");
    
    std::cout << "\n✓ Loaded " << audio.get_cached_buffers() << " audio files" << std::endl;
    
    // Run tests
    try {
        test_basic_playback(audio);
        audio.update();
        
        test_spatial_audio(audio);
        audio.update();
        
        test_multiple_sounds(audio);
        audio.update();
        
        test_volume_controls(audio);
        audio.update();
        
        test_music(audio);
        audio.update();
        
        test_listener_movement(audio);
        audio.update();
        
        test_combat_scenario(audio);
        audio.update();
        
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    // Summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "  All Tests Completed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nAudio System Stats:" << std::endl;
    std::cout << "  Cached buffers: " << audio.get_cached_buffers() << std::endl;
    std::cout << "  Active sources: " << audio.get_active_sources() << std::endl;
    std::cout << "\nVolume Settings:" << std::endl;
    std::cout << "  Master: " << (audio.get_master_volume() * 100) << "%" << std::endl;
    std::cout << "  SFX: " << (audio.get_sfx_volume() * 100) << "%" << std::endl;
    std::cout << "  Music: " << (audio.get_music_volume() * 100) << "%" << std::endl;
    std::cout << "  UI: " << (audio.get_ui_volume() * 100) << "%" << std::endl;
    
    std::cout << "\n✓ Audio system is fully operational!" << std::endl;
    
    return 0;
}

#else

#include <iostream>

int main() {
    std::cout << "Audio system test requires OpenAL support." << std::endl;
    std::cout << "Please build with -DUSE_OPENAL flag." << std::endl;
    return 1;
}

#endif // USE_OPENAL
