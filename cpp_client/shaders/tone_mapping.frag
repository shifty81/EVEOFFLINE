#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hdrTexture;
uniform sampler2D bloomTexture;
uniform bool useBloom;
uniform float exposure;
uniform float gamma;
uniform int toneMapMode; // 0=Reinhard, 1=ACES, 2=Uncharted2

// Reinhard tone mapping
vec3 reinhard(vec3 hdr) {
    return hdr / (hdr + vec3(1.0));
}

// ACES filmic tone mapping (approximate)
vec3 acesFilmic(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Uncharted 2 tone mapping
vec3 uncharted2Tonemap(vec3 x) {
    float A = 0.15;
    float B = 0.50;
    float C = 0.10;
    float D = 0.20;
    float E = 0.02;
    float F = 0.30;
    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

vec3 uncharted2(vec3 color) {
    float exposureBias = 2.0;
    vec3 curr = uncharted2Tonemap(color * exposureBias);
    vec3 W = vec3(11.2);
    vec3 whiteScale = vec3(1.0) / uncharted2Tonemap(W);
    return curr * whiteScale;
}

void main() {
    // Sample HDR color
    vec3 hdrColor = texture(hdrTexture, TexCoord).rgb;
    
    // Add bloom if enabled
    if (useBloom) {
        vec3 bloomColor = texture(bloomTexture, TexCoord).rgb;
        hdrColor += bloomColor;
    }
    
    // Apply exposure
    hdrColor *= exposure;
    
    // Apply tone mapping
    vec3 mapped;
    if (toneMapMode == 0) {
        mapped = reinhard(hdrColor);
    } else if (toneMapMode == 1) {
        mapped = acesFilmic(hdrColor);
    } else {
        mapped = uncharted2(hdrColor);
    }
    
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));
    
    FragColor = vec4(mapped, 1.0);
}
