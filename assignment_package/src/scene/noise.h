#pragma once

#include "glm_includes.h"
namespace Noise {
    float perlinNoise2D(glm::vec2 uv);

    float perlinNoise3D(glm::vec3 p);

    float genPerlinNormal(glm::vec2 uv);

    float fbm2D(float x, float y);

    float fbm1D(float x);

    float random1(glm::vec2 p);
}

