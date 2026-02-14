#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;
uniform float uIntensity;   // 0.0 = no effect, 1.0 = full warp tunnel
uniform float uPhase;       // 1=align, 2=accel, 3=cruise, 4=decel
uniform float uProgress;    // 0.0 - 1.0 overall warp progress
uniform vec2  uDirection;   // screen-space warp direction (normalized)
uniform float uMassNorm;    // 0.0 = frigate, 1.0 = capital (heavier = more distortion)
uniform float uMotionScale; // Accessibility: motion intensity (0.0–1.0)
uniform float uBlurScale;   // Accessibility: blur/distortion intensity (0.0–1.0)

// Warp phase timing constants (matching ShipPhysics phase boundaries)
const float ACCEL_PHASE_FRACTION = 0.33;
const float DECEL_PHASE_START    = 0.67;
const float DECEL_PHASE_DURATION = 0.33;

// Pseudo-random hash
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// ── Layer 1: Radial Distortion ──────────────────────────────────────
// Barrel/pin-cushion distortion centred on screen.
// Heavier ships produce stronger distortion (mass_norm amplifies).
vec2 radialDistortion(vec2 uv, float intensity, float mass) {
    vec2 centre = vec2(0.5);
    vec2 offset = uv - centre;
    float dist  = length(offset);
    float strength = intensity * (0.03 + 0.04 * mass); // 0.03 (frigate) → 0.07 (capital)
    vec2 distorted = centre + offset * (1.0 + strength * dist * dist);
    return distorted;
}

// ── Layer 2: Starfield Velocity Bloom (speed lines) ─────────────────
// Creates radial streaks emanating from centre — the core "speed line" effect.
float speedLines(vec2 uv, float time, float intensity) {
    vec2 centre = vec2(0.5);
    vec2 toEdge = uv - centre;
    float dist = length(toEdge);
    float angle = atan(toEdge.y, toEdge.x);

    float streaks = 0.0;
    for (int i = 0; i < 8; i++) {
        float fi = float(i);
        float seed = hash(vec2(fi * 13.7, fi * 7.3));
        float streakAngle = seed * 6.2831853;
        float angleDiff = abs(mod(angle - streakAngle + 3.14159, 6.2831853) - 3.14159);

        float angularWidth = 0.015 + seed * 0.025;
        float radialSpeed = 2.5 + seed * 4.0;

        float streak = smoothstep(angularWidth, 0.0, angleDiff);
        float radial = fract(dist * 3.5 - time * radialSpeed + seed * 10.0);
        radial = smoothstep(0.0, 0.3, radial) * smoothstep(1.0, 0.5, radial);

        streaks += streak * radial * (0.4 + 0.6 * dist);
    }

    return streaks * intensity;
}

// ── Layer 3: Tunnel Skin (procedural noise band) ────────────────────
// Subtle noise ring around the screen edge, amplified by mass.
float tunnelSkin(vec2 uv, float time, float intensity, float mass) {
    vec2 centre = vec2(0.5);
    float dist = length(uv - centre);
    float angle = atan(uv.y - 0.5, uv.x - 0.5);

    // Ring band at 0.35–0.55 distance from centre
    float ring = smoothstep(0.25, 0.4, dist) * smoothstep(0.65, 0.5, dist);

    // Animated noise pattern
    float noise = hash(vec2(angle * 3.0 + time * 0.5, dist * 8.0 + time * 0.3));
    noise = smoothstep(0.3, 0.7, noise);

    float skinIntensity = 0.3 + 0.3 * mass;  // 0.3 (frigate) → 0.6 (capital)
    return ring * noise * intensity * skinIntensity;
}

// ── Layer 4: Tunnel Vignette ────────────────────────────────────────
// Subtle edge darkening during warp — always present.
float tunnelVignette(vec2 uv, float intensity) {
    vec2 centre = vec2(0.5);
    float dist = length(uv - centre);
    float vignette = smoothstep(0.3, 0.95, dist);
    return vignette * intensity * 0.2;
}

// Blue colour tint during warp — EVE Online uses cool blue, not purple
vec3 warpColorShift(float intensity) {
    vec3 warpColor = vec3(0.2, 0.4, 0.8);
    return mix(vec3(0.0), warpColor, intensity * 0.12);
}

void main() {
    vec2 uv = vUV;

    float effectIntensity = uIntensity;

    // Phase-dependent intensity modulation
    if (uPhase == 1.0) {
        effectIntensity *= 0.1;
    } else if (uPhase == 2.0) {
        effectIntensity *= 0.2 + 0.6 * smoothstep(0.0, 1.0, uProgress / ACCEL_PHASE_FRACTION);
    } else if (uPhase == 3.0) {
        effectIntensity *= 0.8;
    } else if (uPhase == 4.0) {
        float decelProgress = (uProgress - DECEL_PHASE_START) / DECEL_PHASE_DURATION;
        effectIntensity *= 0.8 * (1.0 - smoothstep(0.0, 1.0, decelProgress));
    }

    // Mass-based intensity amplification (heavier ships = more dramatic)
    float massFactor = 1.0 + uMassNorm * 0.4;
    effectIntensity *= massFactor;
    effectIntensity = min(effectIntensity, 1.0);

    // Apply accessibility scaling
    float motionEff = effectIntensity * uMotionScale;
    float blurEff   = effectIntensity * uBlurScale;

    // ── Layer 1: Radial distortion (applied to UV coordinates) ──
    vec2 distortedUV = radialDistortion(uv, blurEff, uMassNorm);

    // ── Layer 2: Starfield velocity bloom (speed lines) ──
    float lines = speedLines(distortedUV, uTime, motionEff);

    // ── Layer 3: Tunnel skin (noise band) ──
    float skin = tunnelSkin(distortedUV, uTime, blurEff, uMassNorm);

    // ── Layer 4: Vignette ──
    float vignette = tunnelVignette(distortedUV, motionEff);

    // Colour composition — bright white-blue for lines, subtle tint
    vec3 lineColor = vec3(0.6, 0.75, 1.0) * lines;
    vec3 skinColor = vec3(0.3, 0.5, 0.9) * skin;
    vec3 tint = warpColorShift(effectIntensity);

    // Combine: additive speed lines + skin + subtle tint
    vec3 color = lineColor + skinColor + tint;
    float alpha = (lines * 0.5 + skin * 0.3) * effectIntensity;

    // Add vignette darkening at edges
    alpha = max(alpha, vignette * 0.3);

    // Flash on warp entry (phase 2 start) and exit (phase 4 end)
    if (uPhase == 2.0 && uProgress < 0.05) {
        float flash = (1.0 - uProgress / 0.05) * 0.4;
        color += vec3(0.6, 0.8, 1.0) * flash;
        alpha = max(alpha, flash * 0.6);
    }
    if (uPhase == 4.0 && uProgress > 0.95) {
        float flash = ((uProgress - 0.95) / 0.05) * 0.35;
        color += vec3(0.7, 0.85, 1.0) * flash;
        alpha = max(alpha, flash * 0.5);
    }

    // Clamp alpha to prevent complete screen coverage
    alpha = min(alpha, 0.6);

    FragColor = vec4(color, alpha);
}
