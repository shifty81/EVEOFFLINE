#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform mat4 invViewProj;

// Procedural noise for nebula clouds
float hash(vec3 p) {
    p = fract(p * vec3(443.897, 441.423, 437.195));
    p += dot(p, p.yzx + 19.19);
    return fract((p.x + p.y) * p.z);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(mix(hash(i + vec3(0,0,0)), hash(i + vec3(1,0,0)), f.x),
            mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x), f.y),
        mix(mix(hash(i + vec3(0,0,1)), hash(i + vec3(1,0,1)), f.x),
            mix(hash(i + vec3(0,1,1)), hash(i + vec3(1,1,1)), f.x), f.y),
        f.z);
}

float fbm(vec3 p) {
    float value = 0.0;
    float amplitude = 0.5;
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(p);
        p *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

void main()
{
    // Reconstruct view direction from screen coordinates
    vec4 clipPos = vec4(TexCoord * 2.0 - 1.0, 1.0, 1.0);
    vec4 worldDir = invViewProj * clipPos;
    vec3 dir = normalize(worldDir.xyz / worldDir.w);

    // Sample nebula at two scales for depth
    vec3 nebulaCoord1 = dir * 3.0;
    vec3 nebulaCoord2 = dir * 1.5 + vec3(10.0, 5.0, 3.0);

    float n1 = fbm(nebulaCoord1);
    float n2 = fbm(nebulaCoord2);

    // Deep space dark nebula — near-black values to eliminate blue tint
    // while maintaining subtle cloud structure visible against pure black
    vec3 nebulaColor1 = vec3(0.04, 0.03, 0.06) * smoothstep(0.35, 0.75, n1);
    // Warm reddish secondary nebula — very faint accent
    vec3 nebulaColor2 = vec3(0.05, 0.02, 0.03) * smoothstep(0.4, 0.8, n2);
    // Subtle teal accent — balanced to avoid any single-color dominance
    vec3 nebulaColor3 = vec3(0.02, 0.04, 0.04) * smoothstep(0.45, 0.85, n1 * n2 * 2.0);

    vec3 color = nebulaColor1 + nebulaColor2 + nebulaColor3;

    // Keep it very subtle so space feels dark and stars remain visible
    float alpha = clamp(length(color) * 1.2, 0.0, 0.25);

    FragColor = vec4(color, alpha);
}
