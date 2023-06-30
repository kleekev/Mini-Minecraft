#include "noise.h"
#include <algorithm>

namespace Noise {

    float random1(glm::vec2 p) {
        return glm::fract(glm::sin((glm::dot(p, glm::vec2(127.1,
                                      311.7)))) *
                     43758.5453);
    }


    glm::vec2 random2(glm::vec2 point) {
        return glm::normalize(
                    glm::fract(
                        glm::sin(
                            glm::vec2(glm::dot(point, glm::vec2(127.1, 311.7)) + 0.143523454f,
                                      glm::dot(point, glm::vec2(269.5, 183.3)) + 0.6523464365f
                                      )
                            ) * 43758.5453f)
                    );
    }

    glm::vec3 random3(glm::vec3 point) {
        return glm::normalize(
                    glm::fract(
                        glm::sin(
                            glm::vec3(glm::dot(point, glm::vec3(127.1, 311.7, 751.f)) + 0.143523454f,
                                      glm::dot(point, glm::vec3(269.5, 183.3, 239.f)) + 0.6523464365f,
                                      glm::dot(point, glm::vec3(420.6, 631.2, 324.f)) + 0.947516243f
                                      )
                            ) * 43758.5453f)
                    );
    }


    float surflet(glm::vec2 P, glm::vec2 gridPoint) {
        // Compute falloff function by converting linear distance to a polynomial
        float distX = abs(P.x - gridPoint.x);
        float distY = abs(P.y - gridPoint.y);
        float tX = 1 - 6 * pow(distX, 5.f) + 15 * pow(distX, 4.f) - 10 * pow(distX, 3.f);
        float tY = 1 - 6 * pow(distY, 5.f) + 15 * pow(distY, 4.f) - 10 * pow(distY, 3.f);
        // Get the random vector for the grid point
        glm::vec2 gradient = random2(gridPoint);
        // Get the vector from the grid point to P
        glm::vec2 diff = P - gridPoint;
        // Get the value of our height field by dotting grid->P with our gradient
        float height = glm::dot(diff, gradient);
        // Scale our height field (i.e. reduce it) by our polynomial falloff function
        return height * tX * tY;
    }

    glm::vec3 pow3(glm::vec3 t, float power) {
        return glm::vec3(pow(t[0], power), pow(t[1], power), pow(t[2], power));
    }

    float surflet(glm::vec3 p, glm::vec3 gridPoint) {
        // Compute the distance between p and the grid point along each axis, and warp it with a
        // quintic function so we can smooth our cells
        glm::vec3 t2 = glm::abs(p - gridPoint);
        glm::vec3 t = glm::vec3(1.f) - 6.f * pow3(t2, 5.f) + 15.f * pow3(t2, 4.f) - 10.f * pow3(t2, 3.f);
        // Get the random vector for the grid point (assume we wrote a function random2
        // that returns a vec2 in the range [0, 1])
        glm::vec3 gradient = random3(gridPoint) * 2.f - glm::vec3(1.f, 1.f, 1.f);
        // Get the vector from the grid point to P
        glm::vec3 diff = p - gridPoint;
        // Get the value of our height field by dotting grid->P with our gradient
        float height = glm::dot(diff, gradient);
        // Scale our height field (i.e. reduce it) by our polynomial falloff function
        return height * t.x * t.y * t.z;
    }


    float perlinNoise2D(glm::vec2 uv) {
        float surfletSum = 0.f;
        // Iterate over the four integer corners surrounding uv
        for(int dx = 0; dx <= 1; ++dx) {
            for(int dy = 0; dy <= 1; ++dy) {
                surfletSum += surflet(uv, glm::floor(uv) + glm::vec2(dx, dy));
            }
        }
        return surfletSum;
    }

    float perlinNoise3D(glm::vec3 p) {
        float surfletSum = 0.f;
        // Iterate over the four integer corners surrounding uv
        for(int dx = 0; dx <= 1; ++dx) {
            for(int dy = 0; dy <= 1; ++dy) {
                for(int dz = 0; dz <= 1; ++dz) {
                    surfletSum += surflet(p, glm::floor(p) + glm::vec3(dx, dy, dz));
                }
            }
        }
        return surfletSum;
    }


    float genPerlinNormal(glm::vec2 uv){
        return (std::clamp((perlinNoise2D(uv) / 0.34f), -1.f, 1.f) + 1.0f)/2.0f;
    }


    float noise2D(glm::vec2 point){
        return glm::normalize(glm::fract(glm::sin(glm::dot(point, glm::vec2(127.1, 311.7)))) * 43758.5453f);
    }


    float interpNoise2D(float x, float y) {
        int intX = int(floor(x));
        float fractX = glm::fract(x);
        int intY = int(floor(y));
        float fractY = glm::fract(y);

        float v1 = noise2D(glm::vec2(intX, intY));
        float v2 = noise2D(glm::vec2(intX + 1, intY));
        float v3 = noise2D(glm::vec2(intX, intY + 1));
        float v4 = noise2D(glm::vec2(intX + 1, intY + 1));

        float i1 = glm::mix(v1, v2, fractX);
        float i2 = glm::mix(v3, v4, fractX);
        return glm::mix(i1, i2, fractY);
    }

    float fbm2D(float x, float y) {
        float total = 0;
        float persistence = 0.5f;
        int octaves = 8;
        float freq = 2.f;
        float amp = 0.5f;
        for(int i = 1; i <= octaves; i++) {
            total += interpNoise2D(x * freq,
                                   y * freq) * amp;

            freq *= 2.f;
            amp *= persistence;
        }
        return total;
    }

    float noise1D(int point){
        return glm::fract(glm::sin(point * 127.1f) * 43758.5453f +0.143523454f);
    }

    float interpNoise1D(float x) {
        int intX = int(floor(x));
        float fractX = glm::fract(x);

        float v1 = noise1D(intX);
        float v2 = noise1D(intX + 1);
        return glm::mix(v1, v2, fractX);
    }

    float fbm1D(float x) {
        float total = 0;
        float persistence = 0.5f;
        int octaves = 8;
        float freq = 2.f;
        float amp = 0.5f;
        for(int i = 1; i <= octaves; i++) {
            total += interpNoise1D(x * freq) * amp;

            freq *= 2.f;
            amp *= persistence;
        }
        return total;
    }

}
