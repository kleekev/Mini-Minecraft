#ifndef UTILS_H
#define UTILS_H
#include "glm_includes.h"
#include "scene/terrain.h"

namespace Utils {
    // Grid March Function borrowed from CIS4600 slides :)
    // Returns boolean on whether or not the grid march intersected a non-empty block
    // Passes out the distance traveled along the ray and the location of the block hit using pointers
    bool gridMarch(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    // same as regular grid march except we ignore nonsolid blocks like water and lava
    bool gridMarchIgnoreNonSolidBlocks(glm::vec3 rayOrigin, glm::vec3 rayDirection, const Terrain &terrain, float *out_dist, glm::ivec3 *out_blockHit);

    bool isSolidBlock(BlockType type);
}

#endif // UTILS_H
