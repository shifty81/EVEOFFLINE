#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D inputTexture;

void main() {
    vec2 texelSize = 1.0 / textureSize(inputTexture, 0);
    
    // 4-tap box filter for downsampling
    vec3 result = vec3(0.0);
    result += texture(inputTexture, TexCoord + vec2(-texelSize.x, -texelSize.y)).rgb;
    result += texture(inputTexture, TexCoord + vec2( texelSize.x, -texelSize.y)).rgb;
    result += texture(inputTexture, TexCoord + vec2(-texelSize.x,  texelSize.y)).rgb;
    result += texture(inputTexture, TexCoord + vec2( texelSize.x,  texelSize.y)).rgb;
    result *= 0.25;
    
    FragColor = vec4(result, 1.0);
}
