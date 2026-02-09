#version 330 core

in vec2 TexCoord;
in vec3 WorldPos;
out vec4 FragColor;

uniform vec4 beamColor;
uniform float intensity; // 0.0 to 1.0
uniform float pulseTime; // For animated pulse effect

void main() {
    // Create beam core and glow effect
    // Distance from center line (0.5 is center in texture coordinates)
    float distFromCenter = abs(TexCoord.y - 0.5) * 2.0;
    
    // Core beam (bright center)
    float core = 1.0 - smoothstep(0.0, 0.3, distFromCenter);
    
    // Outer glow (soft falloff)
    float glow = 1.0 - smoothstep(0.3, 1.0, distFromCenter);
    
    // Combine core and glow
    float alpha = (core * 0.9 + glow * 0.4) * intensity;
    
    // Add pulse animation along beam length
    // Pulse wave: frequency=10.0 cycles along beam, speed=5.0 units/sec, min=0.9, max=1.0
    const float pulseFrequency = 10.0;  // Number of waves along beam
    const float pulseSpeed = 5.0;       // Animation speed
    const float pulseMin = 0.9;         // Minimum brightness
    const float pulseRange = 0.1;       // Brightness variation range
    float pulse = sin(TexCoord.x * pulseFrequency - pulseTime * pulseSpeed) * pulseRange + (pulseMin + pulseRange * 0.5);
    
    // Final color with pulse
    vec3 finalColor = beamColor.rgb * pulse;
    FragColor = vec4(finalColor, alpha * beamColor.a);
}
