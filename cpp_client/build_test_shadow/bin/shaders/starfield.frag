#version 330 core

in float Brightness;

out vec4 FragColor;

void main()
{
    // Create circular point
    vec2 coord = gl_PointCoord - vec2(0.5);
    float distance = length(coord);
    
    if (distance > 0.5) {
        discard;
    }
    
    // Soft falloff for star glow
    float alpha = 1.0 - (distance * 2.0);
    alpha = alpha * alpha;
    
    vec3 starColor = vec3(1.0, 1.0, 1.0) * Brightness;
    FragColor = vec4(starColor, alpha * Brightness);
}
