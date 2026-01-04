#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform sampler2D skyboxTexture;

const float PI = 3.14159265359;

void main()
{    
    // convert cubemap coordinates to spherical mapping
    vec3 dir = normalize(TexCoords);
    
    // Spherical coordinates
    float theta = atan(dir.z, dir.x);
    float phi = asin(dir.y);
    
    // Texture coordinates
    vec2 uv;
    uv.x = theta / (2.0 * PI) + 0.5;
    uv.y = phi / PI + 0.5;
    
    vec3 color = texture(skyboxTexture, uv).rgb;
    FragColor = vec4(color, 1.0);
}
