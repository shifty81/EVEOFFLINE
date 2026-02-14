#ifndef EVE_SYSTEMS_WARP_CINEMATIC_SYSTEM_H
#define EVE_SYSTEMS_WARP_CINEMATIC_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * WarpCinematicSystem — computes per-tick warp tunnel layer intensities,
 * audio profiles, and applies accessibility scaling.
 *
 * Reads WarpState (phase, mass_norm) and WarpAccessibility.
 * Writes WarpTunnelConfig and WarpAudioProfile each tick.
 *
 * Layer intensity rules (per roadmap):
 *   - Radial distortion: phase-dependent, heavier ships = more distortion
 *   - Starfield bloom: ramps up during accel, full during cruise
 *   - Tunnel skin: subtle noise layer, mass-amplified
 *   - Vignette: edge darkening, phase-dependent
 *
 * Audio profile rules:
 *   - Engine core (sub-bass): always on during warp, pitch drops for heavier ships
 *   - Harmonics: ramp during accel/cruise, quieter during align
 *   - Shimmer: environmental layer, gentle during cruise
 */
class WarpCinematicSystem : public ecs::System {
public:
    explicit WarpCinematicSystem(ecs::World* world);
    ~WarpCinematicSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "WarpCinematicSystem"; }

    /**
     * Compute composite intensity from mass_norm and warp phase.
     * Static so it can be tested independently.
     * @param mass_norm  Normalised ship mass (0=frigate, 1=capital)
     * @param phase_frac Phase-dependent fraction (0.0–1.0)
     * @return Composite intensity (0.0–1.0)
     */
    static float computeCompositeIntensity(float mass_norm, float phase_frac);

    /**
     * Compute individual layer intensities from composite intensity.
     */
    static void computeLayers(float composite, float mass_norm,
                              float& radial, float& bloom,
                              float& skin, float& vignette);

    /**
     * Compute audio volumes from composite intensity and mass.
     */
    static void computeAudio(float composite, float mass_norm,
                             float& engine_vol, float& harmonics_vol,
                             float& shimmer_vol,
                             float& engine_pitch, float& harmonics_pitch);
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_WARP_CINEMATIC_SYSTEM_H
