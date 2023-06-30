#include "generation.h"
#include <iostream>
#include <ostream>

namespace Generation{



float genGrasslandsHeight(int x, int z){
    float terrainBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.005, .01 + z * 0.005));
    float hillsBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.015, .01 + z * 0.015));
    float mointainBase = Noise::genPerlinNormal(glm::vec2( + x * 0.004, z * 0.004));

    float mountainNoise = pow(mointainBase, 10.0f);
    float fbm_height = Noise::fbm1D(abs(mointainBase));
    float final_height =  hillsBase * 10.f + mountainNoise * 5.f + fbm_height * 50.f + terrainBase *20.f  +110.f;
    return final_height;
}

float genMountainHeight(int x, int z){
    float terrainBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.005, .01 + z * 0.005));
    float hillsBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.015, .01 + z * 0.015));
    float mointainBase = Noise::genPerlinNormal(glm::vec2( + x * 0.01, z * 0.01));
    float highFrewBase = Noise::genPerlinNormal(glm::vec2( + x * 0.05, z * 0.05));

    float mountainNoise = pow(mointainBase, 10.0f);
    float fbm_height = Noise::fbm1D(abs(highFrewBase));
    float final_height =  glm::min(terrainBase *10.f + hillsBase * 40.f + fbm_height * 30.f +  mountainNoise * 110.f + 140.f, 255.f);
    return final_height;
}

float genSnowHeight(int x, int z){
    float terrainBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.005, .01 + z * 0.005));
    float hillsBase = Noise::genPerlinNormal(glm::vec2(.01 + x * 0.015, .01 + z * 0.015));
    float mointainBase = Noise::genPerlinNormal(glm::vec2( + x * 0.004, z * 0.004));

    float fbm_height = Noise::fbm1D(abs(mointainBase));
    float final_height =  hillsBase * 10.f + fbm_height * 50.f + terrainBase *20.f  +110.f;
    return final_height;
}

float genDesertHeight(int x, int z){
    float desertBase = Noise::genPerlinNormal(glm::vec2( 1.0+ x * 0.006, 1.0+ z * 0.006));

    float fbm_height = Noise::fbm1D(abs(desertBase));
    float final_height =  fbm_height * 50.f  +130.f;
    return final_height;
}


void setBlockAt(Chunk* c, int x, int y, int z, BlockType t){
    c->setBlockAt(x - c->minX, y, z- c->minZ, t);
}

void SetRangeAndGenCaves(Chunk* c, int x, int z, int start, int end, BlockType block){
    for (int i = start; i <= end; i++) {
        // cave generation
        if (i >= 0 && i <= 128 && Noise::perlinNoise3D(glm::vec3(x / 16.f, i / 16.f, z / 16.f)) < 0.f) {
            if (i >= 25) {
                setBlockAt(c, x, i, z, EMPTY);
            }
            else {
                setBlockAt(c, x, i, z, LAVA);
            }
        }
        else {
            setBlockAt(c, x, i, z, block);
        }
    }
}

void SetRange(Chunk* c, int x, int z, int start, int end, BlockType block){
    for (int i = start; i<=end; i++) {
       setBlockAt(c, x, i, z, block);
    }
}

void SetMountain(Chunk* c, int x, int z, float height){
    SetRangeAndGenCaves(c, x, z, 1, height, STONE);
    if(height > 200) {
        setBlockAt(c, x, height, z, SNOW);
    } else {
        setBlockAt(c, x, height, z, STONE);
    }
    if (height < 138) {
        SetRangeAndGenCaves(c, x, z, height, 138, WATER);
    }

}

void SetGrassland(Chunk* c, int x, int z, float height){
    SetRangeAndGenCaves(c, x, z, 1, 128, STONE);
    SetRangeAndGenCaves(c, x, z, 128, height - 1, DIRT);
    setBlockAt(c, x, height, z, GRASS);
    if (height < 138) {
        SetRangeAndGenCaves(c, x, z, height, 138, WATER);
    }
}

void SetDesert(Chunk* c, int x, int z, float height){
    SetRangeAndGenCaves(c, x, z, 1, 128, STONE);
    SetRangeAndGenCaves(c, x, z, 128, height - 1, SAND);
    setBlockAt(c, x, height, z, SAND);
}

void SetSnow(Chunk* c, int x, int z, float height){
    SetRangeAndGenCaves(c, x, z, 1, 128, STONE);
    SetRangeAndGenCaves(c, x, z, 128, height - 1, DIRT);
    setBlockAt(c, x, height, z, SNOW);
    if (height < 138) {
        SetRange(c, x, z, height, 137, WATER);
        setBlockAt(c, x, 138, z, ICE);
    }
}



void GenerateChunk(Chunk* c, int xChunk, int zChunk){

    int xCorner = static_cast<int>(glm::floor(xChunk / 16.f)) *16;
    int zCorner = static_cast<int>(glm::floor(zChunk / 16.f)) *16;

    for (int x=xCorner; x< xCorner + 16; x++){
        for (int z=zCorner; z< zCorner + 16; z++){
            float grassHeight =  genGrasslandsHeight(x,z);
            float mountainHeight =  genMountainHeight(x,z);
            float snowHeight = genSnowHeight(x,z);
            float dessetHeight = genDesertHeight(x,z);

            float temperatureBiome = Noise::genPerlinNormal(glm::vec2(x * 0.001 -.5, z * 0.001 -.5));
            float humidityBiome = Noise::genPerlinNormal(glm::vec2(x * 0.001 +1.0, z * 0.001 +1.0));
            float tempSLERP = glm::smoothstep(0.3f, .7f, temperatureBiome);
            float humiditySLERP = glm::smoothstep(0.3f, .7f, humidityBiome);
            c->setHumidity(humidityBiome);
            float interp1 = glm::mix(mountainHeight, grassHeight, tempSLERP);
            float interp2 = glm::mix(snowHeight, dessetHeight, tempSLERP);
            float finalHeight = glm::mix(interp2, interp1, humiditySLERP);

            if (x ==xCorner+8 && z==zCorner+8){
                c->biome = tempSLERP>.5 ? (humiditySLERP > .5 ? GRASSLAND : DESERT) :
                                          (humiditySLERP > .5 ? MOUNTAIN : SNOWLAND);
            }

            if (tempSLERP>.5){
                if (humiditySLERP > .5){
                    float h = Noise::genPerlinNormal(glm::vec2(x * 0.005, z * 0.005));
                    float humidity = glm::smoothstep(0.1f, 0.9f, h);
                    c->setHumidity(humidity);
                    SetGrassland(c, x, z, finalHeight);
                } else {
                    SetDesert(c, x,z, finalHeight);
                }
            } else {
                if (humiditySLERP > .5){
                    SetMountain(c, x,z, finalHeight);
                } else {
                    SetSnow(c, x,z, finalHeight);
                }
            }
            setBlockAt(c, x, 0, z, BEDROCK);
        }
    }

}


bool CanPlaceTree(Chunk *c, int x, int y, int z){
    if(c->getBlockAt(x,y,z) == GRASS || c->getBlockAt(x,y,z) == DIRT){
        return true;
    } else {
        return false;
    }
}

bool CanPlaceCactus(Chunk *c, int x, int y, int z){
    return c->getBlockAt(x,y,z) == SAND;
}

bool CanPlaceSnowTree(Chunk *c, int x, int y, int z){
    return c->getBlockAt(x,y,z) == SNOW;
}

}
