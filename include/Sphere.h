#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <cmath>

const float PI = 3.14159265359f;

class Sphere {
public:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    Sphere(float radius, unsigned int rings, unsigned int sectors) {
        float const R = 1.0f / (float)(rings - 1);
        float const S = 1.0f / (float)(sectors - 1);

        vertices.resize(rings * sectors * 8); // Position(3) + Normal(3) + TexCoord(2)
        std::vector<float>::iterator v = vertices.begin();

        for(unsigned int r = 0; r < rings; ++r) {
            for(unsigned int s = 0; s < sectors; ++s) {
                float const y = sin(-PI / 2 + PI * r * R);
                float const x = cos(2 * PI * s * S) * sin(PI * r * R);
                float const z = sin(2 * PI * s * S) * sin(PI * r * R);

                // Position
                *v++ = x * radius;
                *v++ = y * radius;
                *v++ = z * radius;

                // Normal
                *v++ = x;
                *v++ = y;
                *v++ = z;
                
                // texture coordinates (flip u to fix mirror issue)
                *v++ = 1.0f - (s * S);
                *v++ = r * R;
            }
        }

        indices.resize(rings * sectors * 6);
        std::vector<unsigned int>::iterator i = indices.begin();

        for(unsigned int r = 0; r < rings - 1; ++r) {
            for(unsigned int s = 0; s < sectors - 1; ++s) {
                *i++ = r * sectors + s;
                *i++ = r * sectors + (s + 1);
                *i++ = (r + 1) * sectors + (s + 1);

                *i++ = r * sectors + s;
                *i++ = (r + 1) * sectors + (s + 1);
                *i++ = (r + 1) * sectors + s;
            }
        }
    }
};

#endif
