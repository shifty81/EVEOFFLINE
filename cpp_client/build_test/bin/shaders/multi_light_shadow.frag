#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Color;
in vec4 FragPosLightSpace;  // Fragment position in light space

out vec4 FragColor;

// Maximum number of lights (can be adjusted)
#define MAX_DIR_LIGHTS 4
#define MAX_POINT_LIGHTS 8
#define MAX_SPOT_LIGHTS 4

// Directional light structure
struct DirLight {
    vec3 direction;
    vec3 color;
};

// Point light structure
struct PointLight {
    vec3 position;
    vec3 color;
    
    float constant;
    float linear;
    float quadratic;
};

// Spot light structure
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    
    float cutoff;
    float outerCutoff;
    
    float constant;
    float linear;
    float quadratic;
};

// Light arrays
uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// Light counts
uniform int numDirLights;
uniform int numPointLights;
uniform int numSpotLights;

// Ambient lighting
uniform vec3 ambientLight;

// Camera position for specular
uniform vec3 viewPos;

// Shadow mapping
uniform sampler2D shadowMap;
uniform bool useShadows;
uniform float shadowBias;

// Calculate shadow factor (0.0 = full shadow, 1.0 = no shadow)
float calculateShadow(vec4 fragPosLightSpace, vec3 normal, vec3 lightDir) {
    if (!useShadows) {
        return 1.0;
    }
    
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    // Check if fragment is outside light frustum
    if (projCoords.z > 1.0) {
        return 1.0;
    }
    
    // Get depth from shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    
    // Get current fragment depth
    float currentDepth = projCoords.z;
    
    // Calculate bias based on surface angle to light
    float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), shadowBias * 0.1);
    
    // PCF (Percentage Closer Filtering) for softer shadows
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // Return inverse (1.0 = fully lit, 0.0 = fully shadowed)
    return 1.0 - shadow;
}

// Calculate directional light contribution
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 albedo, float shadow) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * light.color * 0.5;
    
    // Apply shadow
    return (diffuse + specular) * albedo * shadow;
}

// Calculate point light contribution
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    
    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    vec3 specular = spec * light.color * 0.5;
    
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                               light.quadratic * (distance * distance));
    
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (diffuse + specular) * albedo;
}

// Calculate spot light contribution
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Spot intensity
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
    
    if (intensity > 0.0) {
        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * light.color;
        
        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
        vec3 specular = spec * light.color * 0.5;
        
        // Attenuation
        float distance = length(light.position - fragPos);
        float attenuation = 1.0 / (light.constant + light.linear * distance + 
                                   light.quadratic * (distance * distance));
        
        diffuse *= attenuation * intensity;
        specular *= attenuation * intensity;
        
        return (diffuse + specular) * albedo;
    }
    
    return vec3(0.0);
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 albedo = Color;
    
    // Start with ambient
    vec3 result = ambientLight * albedo;
    
    // Calculate shadow for first directional light only
    float shadow = 1.0;
    if (numDirLights > 0) {
        vec3 lightDir = normalize(-dirLights[0].direction);
        shadow = calculateShadow(FragPosLightSpace, norm, lightDir);
    }
    
    // Add directional lights (first light uses shadows)
    for (int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        float lightShadow = (i == 0) ? shadow : 1.0;  // Only first light casts shadows
        result += calcDirLight(dirLights[i], norm, viewDir, albedo, lightShadow);
    }
    
    // Add point lights (no shadows for now)
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, albedo);
    }
    
    // Add spot lights (no shadows for now)
    for (int i = 0; i < numSpotLights && i < MAX_SPOT_LIGHTS; i++) {
        result += calcSpotLight(spotLights[i], norm, FragPos, viewDir, albedo);
    }
    
    FragColor = vec4(result, 1.0);
}
