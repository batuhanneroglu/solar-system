#ifndef RING_H
#define RING_H

#include <vector>
#include <cmath>

const float PI_RING = 3.14159265359f;

class Ring {
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Ring(float innerRadius, float outerRadius, unsigned int segments) {
        // Create a flat ring in XZ plane
        // Vertices: Position(3) + Normal(3) + TexCoord(2)
        
        vertices.resize((segments + 1) * 2 * 8);
        std::vector<float>::iterator v = vertices.begin();

        for (unsigned int i = 0; i <= segments; ++i) {
            float angle = (2.0f * PI_RING * i) / segments;
            float cosAngle = cos(angle);
            float sinAngle = sin(angle);

            // Inner vertex
            *v++ = innerRadius * cosAngle;  // x
            *v++ = 0.0f;                     // y
            *v++ = innerRadius * sinAngle;   // z
            *v++ = 0.0f;                     // normal x
            *v++ = 1.0f;                     // normal y (pointing up)
            *v++ = 0.0f;                     // normal z
            *v++ = 0.0f;                     // u (inner edge)
            *v++ = (float)i / segments;      // v

            // Outer vertex
            *v++ = outerRadius * cosAngle;   // x
            *v++ = 0.0f;                     // y
            *v++ = outerRadius * sinAngle;   // z
            *v++ = 0.0f;                     // normal x
            *v++ = 1.0f;                     // normal y (pointing up)
            *v++ = 0.0f;                     // normal z
            *v++ = 1.0f;                     // u (outer edge)
            *v++ = (float)i / segments;      // v
        }

        // Create indices for triangle strip
        indices.resize(segments * 6);
        std::vector<unsigned int>::iterator idx = indices.begin();

        for (unsigned int i = 0; i < segments; ++i) {
            unsigned int innerCurrent = i * 2;
            unsigned int outerCurrent = i * 2 + 1;
            unsigned int innerNext = (i + 1) * 2;
            unsigned int outerNext = (i + 1) * 2 + 1;

            // First triangle
            *idx++ = innerCurrent;
            *idx++ = outerCurrent;
            *idx++ = innerNext;

            // Second triangle
            *idx++ = innerNext;
            *idx++ = outerCurrent;
            *idx++ = outerNext;
        }
    }
};

#endif
