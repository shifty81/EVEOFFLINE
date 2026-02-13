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
    for (int i = 0; i < 6; i++) {
        float fi = float(i);
        float seed = hash(vec2(fi * 13.7, fi * 7.3));
        float streakAngle = seed * 6.2831853;
        float angleDiff = abs(mod(angle - streakAngle + 3.14159, 6.2831853) - 3.14159);

        // Narrow angular width, stretched radially
        float angularWidth = 0.02 + seed * 0.03;
        float radialSpeed = 2.0 + seed * 3.0;

        float streak = smoothstep(angularWidth, 0.0, angleDiff);
        float radial = fract(dist * 3.0 - time * radialSpeed + seed * 10.0);
        radial = smoothstep(0.0, 0.3, radial) * smoothstep(1.0, 0.5, radial);

        streaks += streak * radial * (0.5 + 0.5 * dist);
    }

    return streaks * intensity;
}

// Tunnel vignette — darkens edges during warp for immersion
float tunnelVignette(vec2 uv, float intensity) {
    vec2 centre = vec2(0.5);
    float dist = length(uv - centre);
    float vignette = smoothstep(0.2, 0.9, dist);
    return vignette * intensity * 0.4;
}

// Blue-shift color tint during warp
vec3 warpColorShift(float intensity) {
    // Blend from neutral to deep blue-purple
    vec3 warpColor = vec3(0.15, 0.25, 0.8);
    return mix(vec3(0.0), warpColor, intensity * 0.3);
}

void main() {
    vec2 uv = vUV;

    float effectIntensity = uIntensity;

    // Phase-dependent intensity modulation
    if (uPhase == 1.0) {
        // Align phase: very subtle effect (slight blue tint)
        effectIntensity *= 0.15;
    } else if (uPhase == 2.0) {
        // Acceleration: ramp up effect
        effectIntensity *= 0.3 + 0.7 * smoothstep(0.0, 1.0, uProgress / ACCEL_PHASE_FRACTION);
    } else if (uPhase == 3.0) {
        // Cruise: full tunnel effect
        effectIntensity *= 1.0;
    } else if (uPhase == 4.0) {
        // Deceleration: fade out
        float decelProgress = (uProgress - DECEL_PHASE_START) / DECEL_PHASE_DURATION;
        effectIntensity *= 1.0 - smoothstep(0.0, 1.0, decelProgress);
    }

    // Speed lines (streaking stars)
    float lines = speedLines(uv, uTime, effectIntensity);

    // Tunnel vignette
    float vignette = tunnelVignette(uv, effectIntensity);

    // Color composition
    vec3 lineColor = vec3(0.4, 0.6, 1.0) * lines;  // Blue-white speed lines
    vec3 tint = warpColorShift(effectIntensity);

    // Combine: additive speed lines + subtractive vignette + color tint
    vec3 color = lineColor + tint;
    float alpha = max(lines * 0.7, vignette) * effectIntensity;

    // Flash on warp entry (phase 2 start) and exit (phase 4 end)
    if (uPhase == 2.0 && uProgress < 0.05) {
        float flash = (1.0 - uProgress / 0.05) * 0.6;
        color += vec3(0.5, 0.7, 1.0) * flash;
        alpha = max(alpha, flash);
    }
    if (uPhase == 4.0 && uProgress > 0.95) {
        float flash = ((uProgress - 0.95) / 0.05) * 0.5;
        color += vec3(0.6, 0.8, 1.0) * flash;
        alpha = max(alpha, flash);
    }

    FragColor = vec4(color, alpha);
}
