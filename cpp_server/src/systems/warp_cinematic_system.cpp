#include "systems/warp_cinematic_system.h"
#include "components/game_components.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

WarpCinematicSystem::WarpCinematicSystem(ecs::World* world)
    : ecs::System(world) {}

float WarpCinematicSystem::computeCompositeIntensity(float mass_norm, float phase_frac) {
    // Base intensity from phase, amplified by mass (heavier = more dramatic)
    float mass_factor = 1.0f + mass_norm * 0.4f;   // 1.0 (frigate) → 1.4 (capital)
    return std::clamp(phase_frac * mass_factor, 0.0f, 1.0f);
}

void WarpCinematicSystem::computeLayers(float composite, float mass_norm,
                                         float& radial, float& bloom,
                                         float& skin, float& vignette) {
    // Radial distortion: mass-heavy, full during cruise
    radial = composite * (0.6f + 0.4f * mass_norm);

    // Starfield velocity bloom: strong for all ships
    bloom = composite * 0.9f;

    // Tunnel skin (noise layer): subtle, amplified by mass
    skin = composite * (0.3f + 0.3f * mass_norm);

    // Vignette: always present, edge darkening
    vignette = composite * 0.5f;

    // Clamp all to [0,1]
    radial   = std::clamp(radial,   0.0f, 1.0f);
    bloom    = std::clamp(bloom,    0.0f, 1.0f);
    skin     = std::clamp(skin,     0.0f, 1.0f);
    vignette = std::clamp(vignette, 0.0f, 1.0f);
}

void WarpCinematicSystem::computeAudio(float composite, float mass_norm,
                                        float& engine_vol, float& harmonics_vol,
                                        float& shimmer_vol,
                                        float& engine_pitch, float& harmonics_pitch) {
    // Engine core: always present during warp, heavier ships are louder
    engine_vol = composite * (0.5f + 0.5f * mass_norm);

    // Harmonics: ramp up during warp, moderate volume
    harmonics_vol = composite * 0.7f;

    // Shimmer: gentle environmental layer
    shimmer_vol = composite * 0.3f;

    // Pitch: heavier ships have lower pitch
    engine_pitch    = 1.0f - mass_norm * 0.3f;    // 1.0 (frigate) → 0.7 (capital)
    harmonics_pitch = 1.0f - mass_norm * 0.15f;   // 1.0 → 0.85

    // Clamp volumes
    engine_vol    = std::clamp(engine_vol,    0.0f, 1.0f);
    harmonics_vol = std::clamp(harmonics_vol, 0.0f, 1.0f);
    shimmer_vol   = std::clamp(shimmer_vol,   0.0f, 1.0f);
}

void WarpCinematicSystem::update(float /*delta_time*/) {
    if (!world_) return;

    for (auto* entity : world_->getEntities()) {
        auto* warpState = entity->getComponent<components::WarpState>();
        if (!warpState) continue;

        auto* tunnelCfg = entity->getComponent<components::WarpTunnelConfig>();
        auto* audioCfg  = entity->getComponent<components::WarpAudioProfile>();
        if (!tunnelCfg && !audioCfg) continue;

        // Compute phase fraction from WarpState phase
        float phase_frac = 0.0f;
        switch (warpState->phase) {
            case components::WarpState::WarpPhase::None:
                phase_frac = 0.0f;
                break;
            case components::WarpState::WarpPhase::Align:
                phase_frac = 0.1f;      // Subtle during alignment
                break;
            case components::WarpState::WarpPhase::Entry:
                phase_frac = 0.5f;      // Ramping up
                break;
            case components::WarpState::WarpPhase::Cruise:
                phase_frac = 0.85f;     // Near-full intensity
                break;
            case components::WarpState::WarpPhase::Event:
                phase_frac = 0.9f;      // Anomaly spike
                break;
            case components::WarpState::WarpPhase::Exit:
                phase_frac = 0.3f;      // Fading out
                break;
        }

        float composite = computeCompositeIntensity(warpState->mass_norm, phase_frac);

        // Apply accessibility scaling
        float motion_scale = 1.0f;
        float bass_scale   = 1.0f;
        float blur_scale   = 1.0f;
        auto* access = entity->getComponent<components::WarpAccessibility>();
        if (access) {
            motion_scale = access->motion_intensity;
            bass_scale   = access->bass_intensity;
            blur_scale   = access->blur_intensity;
        }

        if (tunnelCfg) {
            computeLayers(composite, warpState->mass_norm,
                          tunnelCfg->radial_distortion, tunnelCfg->starfield_bloom,
                          tunnelCfg->tunnel_skin, tunnelCfg->vignette);
            // Apply accessibility: motion reduces distortion+bloom, blur reduces skin
            tunnelCfg->radial_distortion *= blur_scale;
            tunnelCfg->starfield_bloom   *= motion_scale;
            tunnelCfg->tunnel_skin       *= blur_scale;
            tunnelCfg->vignette          *= motion_scale;
            tunnelCfg->composite_intensity = composite * motion_scale;
        }

        if (audioCfg) {
            computeAudio(composite, warpState->mass_norm,
                         audioCfg->engine_core_volume, audioCfg->harmonics_volume,
                         audioCfg->shimmer_volume,
                         audioCfg->engine_core_pitch, audioCfg->harmonics_pitch);
            // Apply accessibility: bass scaling on engine core
            audioCfg->engine_core_volume *= bass_scale;
        }
    }
}

} // namespace systems
} // namespace atlas
