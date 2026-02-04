#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

// G-Buffer textures
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

// Camera position
uniform vec3 viewPos;

// Light structure (same as multi_light.frag)
struct Light {
    int type; // 0 = directional, 1 = point, 2 = spot
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
    
    // Attenuation (for point and spot lights)
    float constant;
    float linear;
    float quadratic;
    
    // Spotlight
    float cutOff;
    float outerCutOff;
    
    bool castsShadows;
};

// Maximum 16 lights
#define MAX_LIGHTS 16
uniform Light lights[MAX_LIGHTS];
uniform int numLights;

// Ambient lighting
uniform vec3 ambientColor;
uniform float ambientIntensity;

// Calculate lighting for a single light
vec3 calculateLight(Light light, vec3 fragPos, vec3 normal, vec3 albedo, float specular) {
    vec3 lightDir;
    float attenuation = 1.0;
    
    // Calculate light direction based on type
    if (light.type == 0) { // Directional
        lightDir = normalize(-light.direction);
    } else if (light.type == 1) { // Point
        lightDir = normalize(light.position - fragPos);
        
        // Calculate attenuation
        float distance = length(light.position - fragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    } else if (light.type == 2) { // Spot
        lightDir = normalize(light.position - fragPos);
        
        // Calculate attenuation
        float distance = length(light.position - fragPos);
        attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
        
        // Spotlight intensity
        float theta = dot(lightDir, normalize(-light.direction));
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        attenuation *= intensity;
    }
    
    // Diffuse lighting
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * albedo;
    
    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specularColor = spec * light.color * specular;
    
    // Combine
    return (diffuse + specularColor) * light.intensity * attenuation;
}

void main() {
    // Retrieve data from G-Buffer
    vec3 fragPos = texture(gPosition, TexCoord).rgb;
    vec3 normal = texture(gNormal, TexCoord).rgb;
    vec4 albedoSpec = texture(gAlbedoSpec, TexCoord);
    vec3 albedo = albedoSpec.rgb;
    float specularIntensity = albedoSpec.a;
    
    // Ambient lighting
    vec3 ambient = ambientColor * ambientIntensity * albedo;
    
    // Calculate lighting from all lights
    vec3 lighting = ambient;
    for (int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
        lighting += calculateLight(lights[i], fragPos, normal, albedo, specularIntensity);
    }
    
    // Output final color
    FragColor = vec4(lighting, 1.0);
}
