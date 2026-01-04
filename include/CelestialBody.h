#ifndef CELESTIAL_BODY_H
#define CELESTIAL_BODY_H

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <string>
#include <cmath>

class CelestialBody {
public:
    std::string name;
    float mass;
    float radius;
    float displayRadius;
    glm::vec3 position;
    glm::vec3 velocity;      // velocity.z = yörünge hızı (basit mod)
    glm::vec3 color;
    float rotationSpeed;
    float rotationAngle;
    bool isSun;
    
    // Basit yörünge için
    float orbitRadius;       // Başlangıç mesafesi
    float orbitSpeed;        // Yörünge hızı
    float orbitAngle;        // Mevcut açı
    
    // Texture
    unsigned int textureID;
    bool hasTexture;

    CelestialBody(const std::string& name, float mass, float radius, float displayRadius,
                  const glm::vec3& position, const glm::vec3& velocity, 
                  const glm::vec3& color, float rotationSpeed, bool isSun = false)
        : name(name), mass(mass), radius(radius), displayRadius(displayRadius),
          position(position), velocity(velocity), color(color), 
          rotationSpeed(rotationSpeed), rotationAngle(0.0f), isSun(isSun),
          orbitRadius(glm::length(position)), orbitSpeed(velocity.z), orbitAngle(0.0f),
          textureID(0), hasTexture(false) {}

    void update(float deltaTime, const std::vector<CelestialBody*>& bodies) {
        if (isSun) {
            // Güneş sadece döner
            rotationAngle += rotationSpeed * deltaTime;
            return;
        }

        // Basit dairesel yörünge hareketi (görsel mod)
        orbitAngle += orbitSpeed * deltaTime;
        
        // Yeni pozisyon hesapla (XZ düzleminde dairesel)
        position.x = orbitRadius * cos(orbitAngle);
        position.z = orbitRadius * sin(orbitAngle);
        position.y = 0.0f;
        
        // Kendi ekseni etrafında dönme
        rotationAngle += rotationSpeed * deltaTime;
    }
};

#endif
