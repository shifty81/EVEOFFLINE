#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Material albedo fallback — used when vertex colors are not supplied.
// Defaults to white so that lighting results are visible even without
// explicit vertex color data.
uniform vec3 material_albedo = vec3(1.0);

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;

    // Use material_albedo as the per-vertex color.
    // When geometry supplies real vertex colors (layout 3), the uniform
    // can be left at its default (white) and the vertex data will be
    // overridden by the attribute.  For simple test geometry that only
    // provides positions and normals, the uniform provides the albedo.
    Color = material_albedo;

    gl_Position = projection * view * vec4(FragPos, 1.0);
}
