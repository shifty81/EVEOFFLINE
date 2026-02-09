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
    float pulse = sin(TexCoord.x * 10.0 - pulseTime * 5.0) * 0.1 + 0.9;
    
    // Final color with pulse
    vec3 finalColor = beamColor.rgb * pulse;
    FragColor = vec4(finalColor, alpha * beamColor.a);
}
