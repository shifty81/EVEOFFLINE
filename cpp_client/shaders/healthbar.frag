#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 barColor;
uniform float fillAmount; // 0.0 to 1.0

void main() {
    // Only show the bar up to fillAmount
    if (TexCoord.x > fillAmount) {
        discard;
    }
    
    // Add slight gradient for visual depth
    float gradient = mix(0.85, 1.0, TexCoord.y);
    FragColor = vec4(barColor.rgb * gradient, barColor.a);
}
