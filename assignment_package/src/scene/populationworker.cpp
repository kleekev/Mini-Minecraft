#include "populationworker.h"
#include "scene/generation.h"
#include "scene/noise.h"

populationworker::populationworker(Chunk * m_chunkToPopulate, std::unordered_set<Chunk *> * m_populatedChunks, QMutex * m_chunkPopulationLock)
    : m_chunk(m_chunkToPopulate), m_populatedChunks(m_populatedChunks), m_chunkPopulationLock(m_chunkPopulationLock)
{}

void setNeighbor(Chunk *c, int x, int y, int z, BlockType t, bool override){
    if (override){
        c->setBlockAt(x,y,z,t);
        c->VBOdirty = true;
    } else if (c->getBlockAt(x,y,z) == EMPTY){
        c->setBlockAt(x,y,z,t);
        c->VBOdirty = true;
    }
}

void placeAcrossChunks(Chunk *c, int x, int y, int z, BlockType t, bool override){

    int xCorner = static_cast<int>(glm::floor((x+c->minX) / 16.f)) *16;
    int zCorner = static_cast<int>(glm::floor((z+c->minZ) / 16.f)) *16;

    if (xCorner != c->minX || zCorner != c->minZ){
        if (xCorner != c->minX && zCorner == c->minZ){
            Chunk *neighbor = c->m_neighbors[x < 0 ? XNEG : XPOS];\
            int newX = x<0 ? x+16 : x-16;
            setNeighbor(neighbor,newX,y,z,t, override);
        } else if (xCorner == c->minX && zCorner != c->minZ){
            Chunk *neighbor = c->m_neighbors[z < 0 ? ZNEG : ZPOS];
            int newZ = z<0 ? z+16 : z-16;
            setNeighbor(neighbor,x,y,newZ,t, override);

        } else{
            Chunk *neighbor = c->m_neighbors[x < 0 ? XNEG : XPOS]->m_neighbors[z < 0 ? ZNEG : ZPOS];
            int newX = x<0 ? x+16 : x-16;
            int newZ = z<0 ? z+16 : z-16;
            setNeighbor(neighbor,newX,y,newZ,t, override);
        }
    } else{
        if (override){
            c->setBlockAt(x,y,z,t);
        } else if (c->getBlockAt(x,y,z) == EMPTY){
            c->setBlockAt(x,y,z,t);
        }

    }
}

void populationworker::placeTree(int x, int y, int z){
    int trunkLen = Noise::random1(glm::vec2(x+m_chunk->minX,x+m_chunk->minZ)) *3 +4;

    for(int i=0; i<trunkLen; i++){
        placeAcrossChunks(m_chunk, x, y+i, z, OAK_LOG, true);
    }

    for (int yLeaf =trunkLen-3; yLeaf<trunkLen-1; yLeaf++){
        for (int xLeaf = -2; xLeaf<= +2; xLeaf++){
            for (int zLeaf = -2; zLeaf<= +2; zLeaf++){
                placeAcrossChunks(m_chunk, x+ xLeaf, y+yLeaf, z+ zLeaf, OAK_LEAVES, false);
            }
        }
    }

    for (int xLeaf = -1; xLeaf<= +1; xLeaf++){
        for (int zLeaf = -1; zLeaf<= +1; zLeaf++){
            placeAcrossChunks(m_chunk, x+ xLeaf, y+trunkLen-1, z+ zLeaf, OAK_LEAVES, false);
        }
    }

    for (int zLeaf = -1; zLeaf<= +1; zLeaf++){
        placeAcrossChunks(m_chunk, x, y+trunkLen, z+ zLeaf, OAK_LEAVES, false);
    }

    for (int xLeaf = -1; xLeaf<= +1; xLeaf++){
        placeAcrossChunks(m_chunk, x + xLeaf, y+trunkLen, z, OAK_LEAVES, false);
    }

}

void placeCactus(Chunk *c , int x, int y, int z){
    for (int i =0; i<3; i++){
        c->setBlockAt(x,y+i,z, CACTUS);
    }
}

void placePyramid(Chunk *c , int x, int y, int z){
    const int size = 30;

    for (int height =0; height<size; height++){
        int layersize = size - 2*height;
        for (int len = -layersize/2; len <=layersize/2; len++){
            for (int width = -layersize/2; width <=layersize/2; width++){
                placeAcrossChunks(c, x+ len,y+ height,z+width, SANDSTONE, true);
            }
        }
    }
}

void populationworker::placeSnowTree(int x, int y, int z){

    bool layer1[8][8] = {
        {0,0,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,0},
        {1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1},
        {1,1,1,1,1,1,1,1},
        {0,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,0,0}
    };

    bool layer2[8][8] = {
        {0,0,0,0,0,0,0,0},
        {0,0,1,1,1,1,0,0},
        {0,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,0},
        {0,1,1,1,1,1,1,0},
        {0,0,1,1,1,1,0,0},
        {0,0,0,0,0,0,0,0}
    };

    bool layer3[4][4] = {
        {0,1,1,0},
        {1,1,1,1},
        {1,1,1,1},
        {0,1,1,0}
    };

    int numSegments = Noise::random1(glm::vec2(x+m_chunk->minX,x+m_chunk->minZ)) *6 +2;
    int baseHeight = 6;

    for(int i=0; i<baseHeight +4 * numSegments; i++){
        placeAcrossChunks(m_chunk, x, y+i, z, OAK_LOG, true);
        placeAcrossChunks(m_chunk, x+1, y+i, z, OAK_LOG, true);
        placeAcrossChunks(m_chunk, x, y+i, z+1, OAK_LOG, true);
        placeAcrossChunks(m_chunk, x+1, y+i, z+1, OAK_LOG, true);
    }

    for(int i=0; i< numSegments; i++){
        for (int xpos =0; xpos<8; xpos++){
            for (int zpos =0; zpos<8; zpos++){
                if (layer2[xpos][zpos]){
                    placeAcrossChunks(m_chunk, x-3 + xpos, y+baseHeight +1, z-3 + zpos, OAK_LEAVES, false);
                }
            }
        }
        for (int xpos =0; xpos<8; xpos++){
            for (int zpos =0; zpos<8; zpos++){
                if (layer1[xpos][zpos]){
                    placeAcrossChunks(m_chunk, x-3 + xpos, y+baseHeight +2, z-3 + zpos, OAK_LEAVES, false);
                }
            }
        }
        for (int xpos =0; xpos<8; xpos++){
            for (int zpos =0; zpos<8; zpos++){
                if (layer2[xpos][zpos]){
                    placeAcrossChunks(m_chunk, x-3 + xpos, y+baseHeight +3, z-3 + zpos, OAK_LEAVES, false);
                }
            }
        }
        baseHeight+=4;
    }

    for (int xpos =0; xpos<4; xpos++){
        for (int zpos =0; zpos<4; zpos++){
            if (layer3[xpos][zpos]){
                placeAcrossChunks(m_chunk, x-1 + xpos, y+baseHeight, z-1 + zpos, OAK_LEAVES, false);
            }
        }
    }
}


bool checkNeighborType(Chunk *c, Biome b){
    return c->m_neighbors[XPOS]->biome == b &&
            c->m_neighbors[XNEG]->biome == b &&
            c->m_neighbors[ZPOS]->biome == b &&
            c->m_neighbors[ZNEG]->biome == b &&
            c->m_neighbors[XPOS]->m_neighbors[ZPOS]->biome == b &&
            c->m_neighbors[XPOS]->m_neighbors[ZNEG]->biome == b &&
            c->m_neighbors[XNEG]->m_neighbors[ZPOS]->biome == b &&
            c->m_neighbors[XNEG]->m_neighbors[ZPOS]->biome == b;
}

void populationworker::run(){
    std::unordered_map<Biome, float> probtable = {
        {GRASSLAND, .975},
        {DESERT, .985},
        {SNOWLAND, .999},
    };


    for (int x=0; x<16; x++){
        for (int z=0; z<16; z++){
            int xTotal = m_chunk->minX + x;
            int ztotal = m_chunk->minZ + z;
            float pop_level = Noise::random1(glm::vec2(xTotal,ztotal));

            if (pop_level > .99995){
                int structureHeight = m_chunk->getHeightAt(x,z);
                if (m_chunk->biome == DESERT && checkNeighborType(m_chunk, DESERT)){
                    placePyramid(m_chunk, x,structureHeight, z);
                }
            }
        }
    }

    for (int x=0; x<16; x++){
        for (int z=0; z<16; z++){
            int xTotal = m_chunk->minX + x;
            int ztotal = m_chunk->minZ + z;
            float pop_level = Noise::random1(glm::vec2(xTotal,ztotal));
            if (pop_level>probtable.at(GRASSLAND) && m_chunk->biome == GRASSLAND){
                int treeSpawnBlock = m_chunk->getHeightAt(x,z);
                if(treeSpawnBlock < 220 && Generation::CanPlaceTree(m_chunk, x, treeSpawnBlock, z)){
                    placeTree(x,treeSpawnBlock+1, z);

                }
            } else if (pop_level>probtable.at(DESERT) && m_chunk->biome == DESERT){
                int cactusBlock = m_chunk->getHeightAt(x,z);
                if(cactusBlock < 220 && Generation::CanPlaceCactus(m_chunk, x, cactusBlock, z)){
                    placeCactus(m_chunk, x,cactusBlock+1, z);

                }
            } else if (pop_level> probtable.at(SNOWLAND) && m_chunk->biome == SNOWLAND){
                int treeSpawnBlock = m_chunk->getHeightAt(x,z);
                if(treeSpawnBlock < 180 && Generation::CanPlaceSnowTree(m_chunk, x, treeSpawnBlock, z)){
                    placeSnowTree(x,treeSpawnBlock, z);

                }
            }
        }
    }
    m_chunkPopulationLock->lock();
    m_populatedChunks->insert(m_chunk);
    m_chunkPopulationLock->unlock();
}
