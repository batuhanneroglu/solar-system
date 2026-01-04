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
        // sun is self-illuminating with high brightness
        vec3 sunColor = baseColor * 2.5; // brighter sun
        FragColor = vec4(sunColor, 1.0);
        BrightColor = vec4(sunColor, 1.0); // for bloom effect
    } else {
        // ambient lighting
        float ambientStrength = 0.05;
        vec3 ambient = ambientStrength * vec3(1.0, 1.0, 1.0);
        
        // diffuse lighting
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * vec3(1.0, 1.0, 1.0);
        
        // specular reflection
        float specularStrength = 0.1;
        vec3 viewDir = normalize(viewPos - FragPos);
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * vec3(1.0, 1.0, 1.0);
        
        vec3 result = (ambient + diffuse + specular) * baseColor;
        
        // night lights for earth (city lights)
        if (hasNightTexture) {
            vec3 nightColor = texture(nightTexture, TexCoord).rgb;
            // show city lights on night side
            float nightStrength = 1.0 - smoothstep(-0.1, 0.2, diff);
            result += nightColor * nightStrength * 0.8;
        }
        
        // rim lighting for selected planet (edge glow)
        if (isSelected) {
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 norm = normalize(Normal);
            float rimAmount = 1.0 - max(dot(viewDir, norm), 0.0);
            rimAmount = pow(rimAmount, 3.0); // make edges sharper
            vec3 rimColor = vec3(0.4, 0.6, 1.0) * rimAmount * glowIntensity * 2.0;
            result += rimColor;
        }
        
        FragColor = vec4(result, 1.0);
        
        // extract bright areas for bloom - sun only
        BrightColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
