#version 330 core

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform bool isSun;
uniform bool useTexture;
uniform sampler2D textureSampler;
uniform sampler2D nightTexture;
uniform bool hasNightTexture;
uniform bool isSelected;
uniform float glowIntensity;

void main()
{
    vec3 baseColor = objectColor;
    
    if (useTexture) {
        baseColor = texture(textureSampler, TexCoord).rgb;
    }
    
    if (isSun) {
        // Güneş kendi kendine parlar - yüksek parlaklık
        vec3 sunColor = baseColor * 2.5; // Daha parlak güneş
        FragColor = vec4(sunColor, 1.0);
        BrightColor = vec4(sunColor, 1.0); // Bloom için
    } else {
        // Ambient (ortam ışığı)
        float ambientStrength = 0.05;
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        
        // Diffuse (yayılı ışık)
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        
        // Specular (parlak yansıma)
        float specularStrength = 0.1;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);
        
        vec3 result = (ambient + diffuse + specular) * baseColor;
        
        // Dünya için gece ışıkları (şehir ışıkları)
        if (hasNightTexture) {
            vec3 nightColor = texture(nightTexture, TexCoord).rgb;
            // Gece tarafında şehir ışıkları görünsün
            float nightStrength = 1.0 - smoothstep(-0.1, 0.2, diff);
            result += nightColor * nightStrength * 0.8;
        }
        
        // Seçili gezegen parlasın (glow)
        if (isSelected) {
            result += vec3(0.3, 0.3, 0.5) * glowIntensity;
        }
        
        FragColor = vec4(result, 1.0);
        
        // Parlak alanları bloom için çıkar - sadece güneş ve seçili gezegenler
        if (isSelected) {
            BrightColor = vec4(result * 0.5, 1.0);
        } else {
            BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
}
