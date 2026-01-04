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
    glm::vec3 velocity;      // velocity.z = orbit speed (simple mode)
    glm::vec3 color;
    float rotationSpeed;
    float rotationAngle;
    bool isSun;
    
    // for simple orbit
    float orbitRadius;       // initial distance
    float orbitSpeed;        // orbit speed
    float orbitAngle;        // current angle
    
    // Texture
    unsigned int textureID;
    bool hasTexture;
    
    // Info panel texture (for sidebar display)
    unsigned int infoTextureID;
    bool hasInfoTexture;
    
    // Ring properties
    bool hasRing;
    float ringInnerRadius;
    float ringOuterRadius;
    unsigned int ringTextureID;

    CelestialBody(const std::string& name, float mass, float radius, float displayRadius,
                  const glm::vec3& position, const glm::vec3& velocity, 
                  const glm::vec3& color, float rotationSpeed, bool isSun = false)
        : name(name), mass(mass), radius(radius), displayRadius(displayRadius),
          position(position), velocity(velocity), color(color), 
          rotationSpeed(rotationSpeed), rotationAngle(0.0f), isSun(isSun),
          orbitRadius(glm::length(position)), orbitSpeed(velocity.z), orbitAngle(0.0f),
          textureID(0), hasTexture(false), infoTextureID(0), hasInfoTexture(false),
          hasRing(false), ringInnerRadius(0.0f), ringOuterRadius(0.0f), ringTextureID(0) {}

    void update(float deltaTime, const std::vector<CelestialBody*>& bodies) {
        if (isSun) {
            // sun only rotates
            rotationAngle += rotationSpeed * deltaTime;
            return;
        }

        // simple circular orbit movement (visual mode)
        orbitAngle += orbitSpeed * deltaTime;
        
        // calculate new position (circular in xz plane)
        position.x = orbitRadius * cos(orbitAngle);
        position.z = orbitRadius * sin(orbitAngle);
        position.y = 0.0f;
        
        // rotation around own axis
        rotationAngle += rotationSpeed * deltaTime;
    }
};

#endif
