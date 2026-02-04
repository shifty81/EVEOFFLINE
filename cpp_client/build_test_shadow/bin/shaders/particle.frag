#version 330 core

in vec4 vColor;
out vec4 FragColor;

void main() {
    // Create circular particles with soft edges
    vec2 center = gl_PointCoord - vec2(0.5);
    float dist = length(center);
    
    if (dist > 0.5) {
        discard;
    }
    
    // Soft falloff
    float alpha = 1.0 - smoothstep(0.0, 0.5, dist);
    FragColor = vec4(vColor.rgb, vColor.a * alpha);
}
