#version 330 core

// G-Buffer outputs
layout (location = 0) out vec3 gPosition;  // World position
layout (location = 1) out vec3 gNormal;    // World normal
layout (location = 2) out vec4 gAlbedoSpec; // RGB: albedo, A: specular intensity

// Input from vertex shader
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Material properties
uniform vec3 material_albedo;
uniform float material_specular;
uniform float material_metallic;
uniform float material_roughness;

void main() {
    // Store world position
    gPosition = FragPos;
    
    // Store normalized normal
    gNormal = normalize(Normal);
    
    // Store material properties
    // For now, use uniform material values
    // In a full implementation, these would come from textures
    gAlbedoSpec.rgb = material_albedo;
    gAlbedoSpec.a = material_specular;
}
