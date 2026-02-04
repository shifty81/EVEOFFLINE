#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

// Output to fragment shader
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform position to world space
    vec4 worldPos = model * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    
    // Transform normal to world space (use normal matrix for non-uniform scaling)
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);
    
    TexCoord = aTexCoord;
    
    // Transform to clip space
    gl_Position = projection * view * worldPos;
}
