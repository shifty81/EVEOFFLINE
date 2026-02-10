#version 330 core

in float Brightness;
in float StarSeed;

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
    
    // Color temperature variation based on star seed
    // Creates blue, white, yellow, and occasional red stars
    vec3 starColor;
    if (StarSeed < 0.15) {
        // Hot blue-white stars (O/B class)
        starColor = vec3(0.7, 0.8, 1.0);
    } else if (StarSeed < 0.5) {
        // White/pale yellow stars (A/F class)
        starColor = vec3(1.0, 1.0, 0.95);
    } else if (StarSeed < 0.75) {
        // Yellow stars (G class, like our Sun)
        starColor = vec3(1.0, 0.95, 0.8);
    } else if (StarSeed < 0.9) {
        // Orange stars (K class)
        starColor = vec3(1.0, 0.85, 0.6);
    } else {
        // Red stars (M class)
        starColor = vec3(1.0, 0.6, 0.4);
    }
    
    starColor *= Brightness;
    FragColor = vec4(starColor, alpha * Brightness);
}
