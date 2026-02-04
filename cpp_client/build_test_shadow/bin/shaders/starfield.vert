#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in float aSize;
layout (location = 2) in float aBrightness;

out float Brightness;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    Brightness = aBrightness;
    gl_Position = projection * view * vec4(aPos, 1.0);
    gl_PointSize = aSize;
}
