#version 330 core

in vec2 vUV;
out vec4 FragColor;

uniform float uTime;
uniform float uIntensity;   // 0.0 = no effect, 1.0 = full warp tunnel
uniform float uPhase;       // 1=align, 2=accel, 3=cruise, 4=decel
uniform float uProgress;    // 0.0 - 1.0 overall warp progress
uniform vec2  uDirection;   // screen-space warp direction (normalized)

// Warp phase timing constants (matching ShipPhysics phase boundaries)
const float ACCEL_PHASE_FRACTION = 0.33;
const float DECEL_PHASE_START    = 0.67;
const float DECEL_PHASE_DURATION = 0.33;

// Pseudo-random hash
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

// Streaking star lines — creates the speed-line tunnel effect
float speedLines(vec2 uv, float time, float intensity) {
    vec2 centre = vec2(0.5);
    vec2 toEdge = uv - centre;
    float dist = length(toEdge);
    float angle = atan(toEdge.y, toEdge.x);

    // Radial streaks emanating from center
    float streaks = 0.0;
    for (int i = 0; i < 8; i++) {
        float fi = float(i);
        float seed = hash(vec2(fi * 13.7, fi * 7.3));
        float streakAngle = seed * 6.2831853;
        float angleDiff = abs(mod(angle - streakAngle + 3.14159, 6.2831853) - 3.14159);

        // Narrow angular width, stretched radially
        float angularWidth = 0.015 + seed * 0.025;
        float radialSpeed = 2.5 + seed * 4.0;

        float streak = smoothstep(angularWidth, 0.0, angleDiff);
        float radial = fract(dist * 3.5 - time * radialSpeed + seed * 10.0);
        radial = smoothstep(0.0, 0.3, radial) * smoothstep(1.0, 0.5, radial);

        streaks += streak * radial * (0.4 + 0.6 * dist);
    }

    return streaks * intensity;
}

// Tunnel vignette — subtle edge darkening during warp
float tunnelVignette(vec2 uv, float intensity) {
    vec2 centre = vec2(0.5);
    float dist = length(uv - centre);
    float vignette = smoothstep(0.3, 0.95, dist);
    return vignette * intensity * 0.2;
}

// Blue color tint during warp — EVE Online uses cool blue, not purple
vec3 warpColorShift(float intensity) {
    vec3 warpColor = vec3(0.2, 0.4, 0.8);  // Cool blue (not purple)
    return mix(vec3(0.0), warpColor, intensity * 0.12);
}

void main() {
    vec2 uv = vUV;

    float effectIntensity = uIntensity;

    // Phase-dependent intensity modulation
    if (uPhase == 1.0) {
        // Align phase: very subtle effect (slight blue tint)
        effectIntensity *= 0.1;
    } else if (uPhase == 2.0) {
        // Acceleration: ramp up effect
        effectIntensity *= 0.2 + 0.6 * smoothstep(0.0, 1.0, uProgress / ACCEL_PHASE_FRACTION);
    } else if (uPhase == 3.0) {
        // Cruise: moderate tunnel effect (not overwhelming)
        effectIntensity *= 0.8;
    } else if (uPhase == 4.0) {
        // Deceleration: fade out
        float decelProgress = (uProgress - DECEL_PHASE_START) / DECEL_PHASE_DURATION;
        effectIntensity *= 0.8 * (1.0 - smoothstep(0.0, 1.0, decelProgress));
    }

    // Speed lines (streaking stars)
    float lines = speedLines(uv, uTime, effectIntensity);

    // Tunnel vignette
    float vignette = tunnelVignette(uv, effectIntensity);

    // Color composition — use bright white-blue for lines, minimal tint
    vec3 lineColor = vec3(0.6, 0.75, 1.0) * lines;  // Bright blue-white speed lines
    vec3 tint = warpColorShift(effectIntensity);

    // Combine: additive speed lines + subtle tint (no purple saturation)
    vec3 color = lineColor + tint;
    float alpha = lines * 0.5 * effectIntensity;

    // Add subtle vignette darkening at edges only
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
