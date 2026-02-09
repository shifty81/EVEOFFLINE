#version 330 core

layout (location = 0) in vec2 aPos;

out vec2 TexCoord;

void main()
{
    TexCoord = aPos * 0.5 + 0.5;
    gl_Position = vec4(aPos, 0.999, 1.0);
}
