#version 330 core
out vec4 FragColor;
out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D ringTexture;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool useTexture;
uniform vec3 ringColor;

void main()
{
    // Get base color from texture or use solid color
    vec4 texColor;
    if (useTexture) {
        texColor = texture(ringTexture, TexCoord);
        // If texture has transparency, use it
        if (texColor.a < 0.1)
            discard;
    } else {
        texColor = vec4(ringColor, 0.7);
    }
    
    // Simple lighting
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 norm = normalize(Normal);
    
    // Diffuse lighting
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Ambient lighting
    float ambient = 0.3;
    
    // Calculate distance-based attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.0001 * distance + 0.000001 * distance * distance);
    
    // Combine lighting
    float lighting = ambient + diff * attenuation;
    lighting = clamp(lighting, 0.2, 1.0);
    
    // Apply lighting to texture color
    vec3 result = texColor.rgb * lighting;
    
    FragColor = vec4(result, texColor.a);
    
    // No bloom for rings
    BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
}
