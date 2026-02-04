#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;
uniform float filterRadius;

void main() {
    // 9-tap tent filter for upsampling
    vec2 texelSize = 1.0 / textureSize(inputTexture, 0);
    float x = texelSize.x * filterRadius;
    float y = texelSize.y * filterRadius;
    
    // Center sample
    vec3 result = texture(inputTexture, TexCoord).rgb * 4.0;
    
    // Adjacent samples
    result += texture(inputTexture, TexCoord + vec2(-x,  0.0)).rgb * 2.0;
    result += texture(inputTexture, TexCoord + vec2( x,  0.0)).rgb * 2.0;
    result += texture(inputTexture, TexCoord + vec2( 0.0, -y)).rgb * 2.0;
    result += texture(inputTexture, TexCoord + vec2( 0.0,  y)).rgb * 2.0;
    
    // Corner samples
    result += texture(inputTexture, TexCoord + vec2(-x, -y)).rgb;
    result += texture(inputTexture, TexCoord + vec2( x, -y)).rgb;
    result += texture(inputTexture, TexCoord + vec2(-x,  y)).rgb;
    result += texture(inputTexture, TexCoord + vec2( x,  y)).rgb;
    
    result /= 16.0;
    
    FragColor = vec4(result, 1.0);
}
