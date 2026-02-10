#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;
layout (location = 2) in float aBrightness;

out float Brightness;
out float StarSeed;  // Unique seed per star for color/twinkle

uniform mat4 view;
uniform mat4 projection;
uniform float time;  // For twinkling animation

void main()
{
    Brightness = aBrightness;
    // Use position hash as unique seed for each star
    StarSeed = fract(aPos.x * 12.9898 + aPos.y * 78.233 + aPos.z * 45.164);
    
    gl_Position = projection * view * vec4(aPos, 1.0);
    
    // Slight size variation with twinkling
    float twinkle = 1.0 + 0.3 * sin(time * (2.0 + StarSeed * 4.0) + StarSeed * 6.2831);
    gl_PointSize = aSize * twinkle;
}
