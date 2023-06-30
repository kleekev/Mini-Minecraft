#include "terrain.h"
#include "cube.h"
#include <stdexcept>
#include <iostream>
#include "noise.h"
#include "blocktypeworker.h"
#include "scene/populationworker.h"
#include "scene/vboworker.h"
#include <QThreadPool>


#define RENDERSIZE 3
#define GENSIZE (RENDERSIZE+1)

Terrain::Terrain(OpenGLContext *context)
    : m_chunks(), m_generatedTerrain(), mp_context(context)
{}

Terrain::~Terrain() {

}

// Combine two 32-bit ints into one 64-bit int
// where the upper 32 bits are X and the lower 32 bits are Z
int64_t toKey(int x, int z) {
    int64_t xz = 0xffffffffffffffff;
    int64_t x64 = x;
    int64_t z64 = z;

    // Set all lower 32 bits to 1 so we can & with Z later
    xz = (xz & (x64 << 32)) | 0x00000000ffffffff;

    // Set all upper 32 bits to 1 so we can & with XZ
    z64 = z64 | 0xffffffff00000000;

    // Combine
    xz = xz & z64;
    return xz;
}

glm::ivec2 toCoords(int64_t k) {
    // Z is lower 32 bits
    int64_t z = k & 0x00000000ffffffff;
    // If the most significant bit of Z is 1, then it's a negative number
    // so we have to set all the upper 32 bits to 1.
    // Note the 8    V
    if(z & 0x0000000080000000) {
        z = z | 0xffffffff00000000;
    }
    int64_t x = (k >> 32);

    return glm::ivec2(x, z);
}

// Surround calls to this with try-catch if you don't know whether
// the coordinates at x, y, z have a corresponding Chunk
BlockType Terrain::getBlockAt(int x, int y, int z) const
{
    if(hasChunkAt(x, z)) {
        // Just disallow action below or above min/max height,
        // but don't crash the game over it.
        if(y < 0 || y >= 256) {
            return EMPTY;
        }
        const uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        return c->getBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                             static_cast<unsigned int>(y),
                             static_cast<unsigned int>(z - chunkOrigin.y));
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

BlockType Terrain::getBlockAt(glm::vec3 p) const {
    return getBlockAt(p.x, p.y, p.z);
}

bool Terrain::hasChunkAt(int x, int z) const {
    // Map x and z to their nearest Chunk corner
    // By flooring x and z, then multiplying by 16,
    // we clamp (x, z) to its nearest Chunk-space corner,
    // then scale back to a world-space location.
    // Note that floor() lets us handle negative numbers
    // correctly, as floor(-1 / 16.f) gives us -1, as
    // opposed to (int)(-1 / 16.f) giving us 0 (incorrect!).
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.find(toKey(16 * xFloor, 16 * zFloor)) != m_chunks.end();
}


uPtr<Chunk>& Terrain::getChunkAt(int x, int z) {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks[toKey(16 * xFloor, 16 * zFloor)];
}


const uPtr<Chunk>& Terrain::getChunkAt(int x, int z) const {
    int xFloor = static_cast<int>(glm::floor(x / 16.f));
    int zFloor = static_cast<int>(glm::floor(z / 16.f));
    return m_chunks.at(toKey(16 * xFloor, 16 * zFloor));
}

Chunk* Terrain::instantiateChunkAt(int x, int z) {
    uPtr<Chunk> chunk = mkU<Chunk>(mp_context, x, z);
    Chunk *cPtr = chunk.get();
    m_chunks[toKey(x, z)] = std::move(chunk);
    // Set the neighbor pointers of itself and its neighbors
    if(hasChunkAt(x, z + 16)) {
        auto &chunkNorth = m_chunks[toKey(x, z + 16)];
        cPtr->linkNeighbor(chunkNorth, ZPOS);
    }
    if(hasChunkAt(x, z - 16)) {
        auto &chunkSouth = m_chunks[toKey(x, z - 16)];
        cPtr->linkNeighbor(chunkSouth, ZNEG);
    }
    if(hasChunkAt(x + 16, z)) {
        auto &chunkEast = m_chunks[toKey(x + 16, z)];
        cPtr->linkNeighbor(chunkEast, XPOS);
    }
    if(hasChunkAt(x - 16, z)) {
        auto &chunkWest = m_chunks[toKey(x - 16, z)];
        cPtr->linkNeighbor(chunkWest, XNEG);
    }
    return cPtr;
}

// TODO: When you make Chunk inherit from Drawable, change this code so
// it draws each Chunk with the given ShaderProgram, remembering to set the
// model matrix to the proper X and Z translation!
void Terrain::draw(ShaderProgram *shaderProgram) {
    int xCorner = static_cast<int>(glm::floor(playerCoords[0] / 64.f)) *64;
    int zCorner = static_cast<int>(glm::floor(playerCoords[2] / 64.f)) *64;

    for (int x = xCorner - RENDERSIZE *64; x <= xCorner + (RENDERSIZE+1) *64; x+=16){
        for (int z = zCorner - RENDERSIZE *64; z <= zCorner + (RENDERSIZE+1) *64; z+=16){
            if (hasChunkAt(x,z)){
                Chunk * c = getChunkAt(x,z).get();
                if (c->VBOready){
                    shaderProgram->drawInterleaved(*c, 0, false);
                }
            }
        }
    }

    for (int x = xCorner - RENDERSIZE *64; x <= xCorner + (RENDERSIZE+1) *64; x+=16){
        for (int z = zCorner - RENDERSIZE *64; z <= zCorner + (RENDERSIZE+1) *64; z+=16){
            if (hasChunkAt(x,z)){
                Chunk * c = getChunkAt(x,z).get();
                if (c->VBOready){
                    shaderProgram->drawInterleaved(*c, 0, true);
                }
            }
        }
    }
}

void Terrain::checkVBOState(Chunk *c ){
    if (c->VBOdirty == true && c->VBOState != VBO_RUNNING){
        m_VBOGenerationQueue.insert(c);
        c->VBOState = VBO_WAITING;
        c->VBOdirty = false;
    } else if (c->VBOState == VBO_NONE){
        m_VBOGenerationQueue.insert(c);
    }
}

void Terrain::updateGenerationThreads(){
    m_TerrainGenLock.lock();

    for (auto it = m_generatedChunks.begin(); it != m_generatedChunks.end(); ) {
        Chunk *c = (*it);
        c->genState = TERRAIN_DONE;
        it = m_generatedChunks.erase(it);
        m_PopulatonQueue.insert(c);
    }
    m_TerrainGenLock.unlock();

    for(auto it = m_PopulatonQueue.begin(); it != m_PopulatonQueue.end(); ) {
        Chunk *c = (*it);
        if (checkNeighborStatusPopulation(c)){
            spanwnPopulationWorker(c);
            it = m_PopulatonQueue.erase(it);
        } else{
            it++;
        }
    }

    m_PopulationGenLock.lock();
    for (auto it = m_populatedChunks.begin(); it != m_populatedChunks.end(); ) {
        Chunk *c = (*it);
        c->genState = GEN_COMPLETE;
        it = m_populatedChunks.erase(it);
    }
    m_PopulationGenLock.unlock();

}

void Terrain::setBlockAt(int x, int y, int z, BlockType t)
{
    if(hasChunkAt(x, z)) {
        uPtr<Chunk> &c = getChunkAt(x, z);
        glm::vec2 chunkOrigin = glm::vec2(floor(x / 16.f) * 16, floor(z / 16.f) * 16);
        c->setBlockAt(static_cast<unsigned int>(x - chunkOrigin.x),
                      static_cast<unsigned int>(y),
                      static_cast<unsigned int>(z - chunkOrigin.y),
                      t);
        c->VBOdirty = true;

        // if x, y is at the boundary of a chunk, redraw the neighboring chunk as well
        if (x % 16 == 0 && hasChunkAt(x - 1, z) && getBlockAt(x - 1, y, z) != EMPTY) {
            getChunkAt(chunkOrigin[0] - 16, chunkOrigin[1])->VBOdirty = true;
        }
        if ((x+1) % 16 == 0  && hasChunkAt(x + 1, z) && getBlockAt(x + 1, y, z) != EMPTY) {
            getChunkAt(chunkOrigin[0] + 16, chunkOrigin[1])->VBOdirty = true;
        }
        if (z % 16 == 0  && hasChunkAt(x, z - 1) && getBlockAt(x, y, z - 1) != EMPTY) {
            getChunkAt(chunkOrigin[0], chunkOrigin[1] - 16)->VBOdirty = true;
        }
        if ((z + 1) % 16 == 0  && hasChunkAt(x, z + 1) && getBlockAt(x, y, z + 1) != EMPTY) {
            getChunkAt(chunkOrigin[0], chunkOrigin[1] + 16)->VBOdirty = true;
        }
    }
    else {
        throw std::out_of_range("Coordinates " + std::to_string(x) +
                                " " + std::to_string(y) + " " +
                                std::to_string(z) + " have no Chunk!");
    }
}

void Terrain::spanwGenerationWorker(int64_t terrainGenZone){
    m_generatedTerrain.insert(terrainGenZone);
    std::vector<Chunk *> chunksToGen;
    glm::ivec2 coords = toCoords(terrainGenZone);
    for(int x = coords.x; x< coords.x +64; x += 16){
        for(int z = coords.y; z< coords.y +64; z += 16){
            Chunk * c = instantiateChunkAt(x, z);
            c->genState = TERRAIN_RUNNING;
            chunksToGen.push_back(c);
        }
    }

    BlockTypeWorker * worker = new BlockTypeWorker(coords.x, coords.y, chunksToGen,
                                                   &m_generatedChunks, &m_TerrainGenLock);
    QThreadPool::globalInstance()->start(worker);

}

void Terrain::spanwnPopulationWorker(Chunk * chunk){

    populationworker * worker = new populationworker(chunk,&m_populatedChunks, &m_PopulationGenLock);
    chunk->genState = POPULATION_RUNNING;
    QThreadPool::globalInstance()->start(worker);

}

void Terrain::spawnVBOWorker(Chunk * chunk){
    VBOWorker * worker = new VBOWorker(chunk, &m_VBOChunks, &m_VBOCompletedLock);
    chunk->VBOState = VBO_RUNNING;
    QThreadPool::globalInstance()->start(worker);
}

void Terrain::updateVBOThreads(){
    m_VBOCompletedLock.lock();

    for (auto it = m_VBOChunks.begin(); it != m_VBOChunks.end(); ) {
        (*it)->SendVBOdata();
        it = m_VBOChunks.erase(it);
    }
    m_VBOCompletedLock.unlock();

}

bool Terrain::checkNeighborStatus(Chunk * c, GenState status){
    return c->genState == status && ((c->m_neighbors[XPOS] != nullptr) ?  c->m_neighbors[XPOS]->genState ==  status:  true)
            && ((c->m_neighbors[XNEG] != nullptr) ?  c->m_neighbors[XNEG]->genState == status :  true)
            && ((c->m_neighbors[ZPOS] != nullptr) ?  c->m_neighbors[ZPOS]->genState == status :  true)
            && ((c->m_neighbors[ZNEG] != nullptr) ?  c->m_neighbors[ZNEG]->genState == status :  true);
}

bool Terrain::checkNeighborStatusPopulation(Chunk * c){
    bool first =   ((c->m_neighbors[XPOS] != nullptr) ?  c->m_neighbors[XPOS]->genState ==  TERRAIN_DONE || c->m_neighbors[XPOS]->genState ==  GEN_COMPLETE:  false)
            && ((c->m_neighbors[XNEG] != nullptr) ?  c->m_neighbors[XNEG]->genState == TERRAIN_DONE || c->m_neighbors[XNEG]->genState ==  GEN_COMPLETE :  false)
            && ((c->m_neighbors[ZPOS] != nullptr) ?  c->m_neighbors[ZPOS]->genState == TERRAIN_DONE || c->m_neighbors[ZPOS]->genState ==  GEN_COMPLETE :  false)
            && ((c->m_neighbors[ZNEG] != nullptr) ?  c->m_neighbors[ZNEG]->genState == TERRAIN_DONE || c->m_neighbors[ZNEG]->genState ==  GEN_COMPLETE :  false);
    if (first){
            return ((c->m_neighbors[XNEG]->m_neighbors[ZPOS] != nullptr) ?  c->m_neighbors[XNEG]->m_neighbors[ZPOS]->genState == TERRAIN_DONE || c->m_neighbors[XNEG]->m_neighbors[ZPOS]->genState ==  GEN_COMPLETE :  false)
            && ((c->m_neighbors[XNEG]->m_neighbors[ZNEG] != nullptr) ?  c->m_neighbors[XNEG]->m_neighbors[ZNEG]->genState == TERRAIN_DONE || c->m_neighbors[XNEG]->m_neighbors[ZNEG]->genState ==  GEN_COMPLETE :  false)
            && ((c->m_neighbors[XPOS]->m_neighbors[ZPOS] != nullptr) ?  c->m_neighbors[XPOS]->m_neighbors[ZPOS]->genState == TERRAIN_DONE || c->m_neighbors[XPOS]->m_neighbors[ZPOS]->genState ==  GEN_COMPLETE :  false)
            && ((c->m_neighbors[XPOS]->m_neighbors[ZNEG] != nullptr) ?  c->m_neighbors[XPOS]->m_neighbors[ZNEG]->genState == TERRAIN_DONE || c->m_neighbors[XPOS]->m_neighbors[ZNEG]->genState ==  GEN_COMPLETE :  false);
    }
    return false;
}

void Terrain::updateVBOGenQueue(){

    for(auto it = m_VBODeletionQueue.begin(); it != m_VBODeletionQueue.end(); ) {
        Chunk *c = (*it);
        if (c->VBOState == VBO_DONE){
            c->deleteVBOdata();
            it = m_VBODeletionQueue.erase(it);
        } else if(c->VBOState == VBO_WAITING){
            m_VBOGenerationQueue.erase(c);
            c->VBOState = VBO_NONE;
            if (c->VBOready){
                c->deleteVBOdata();
            }
            it = m_VBODeletionQueue.erase(it);
        } else if (c->VBOState == VBO_RUNNING){
            ++it;
        } else {
            throw std::logic_error("continuty error in VBO deletion Queue");
        }
    }

    for(auto it = m_VBOGenerationQueue.begin(); it != m_VBOGenerationQueue.end(); ) {
        Chunk *c = (*it);
        if (checkNeighborStatus(c, GEN_COMPLETE) && c->VBOState != VBO_RUNNING){
            spawnVBOWorker(c);
            it = m_VBOGenerationQueue.erase(it);
        } else {
            ++it;
        }
    }
}

glm::vec2 Terrain::IntertiaCenter(glm::vec3 playerPos){
    glm::vec2 pos2d = glm::vec2(playerPos[0], playerPos[2]);
    if (glm::distance2(pos2d, m_playerInertia) > 32){
        m_playerInertia = glm::normalize(m_playerInertia - pos2d) * 32.f + pos2d;
    }
    return m_playerInertia;
}

void Terrain::GenerateNew(glm::vec3 playerPos){

    playerCoords = playerPos;
    int xCorner = static_cast<int>(glm::floor(playerPos[0] / 64.f)) *64;
    int zCorner = static_cast<int>(glm::floor(playerPos[2] / 64.f)) *64;

    for (int x = xCorner - GENSIZE * 64; x <= xCorner + (GENSIZE+1) *64; x+=64){
        for (int z = zCorner - GENSIZE * 64; z <= zCorner + (GENSIZE+1) *64; z+=64){
            int64_t key =  toKey(x, z);
            if(m_generatedTerrain.count(key) == 0){
                spanwGenerationWorker(key);

            }
        }
    }

    glm::vec2 intertiaCenter = IntertiaCenter(playerPos);

    std::unordered_map<int64_t, Chunk *> currDrawChunks;

    xCorner = static_cast<int>(glm::floor(intertiaCenter[0] / 16.f)) *16;
    zCorner = static_cast<int>(glm::floor(intertiaCenter[1] / 16.f)) *16;

    for (int x = xCorner - GENSIZE * 64; x <= xCorner + (GENSIZE+1) *64; x+=16){
        for (int z = zCorner - GENSIZE * 64; z <= zCorner + (GENSIZE+1) *64; z+=16){
            if (hasChunkAt(x,z)){
                int64_t key =  toKey(x, z);
                Chunk * c = getChunkAt(x,z).get();
                currDrawChunks[key]= c;
                checkVBOState(c);
                if(m_chunksLastGen.find(key) != m_chunksLastGen.end()){
                    m_chunksLastGen.erase(key);
                }
            }
        }
    }

    for (auto it = m_chunksLastGen.begin(); it != m_chunksLastGen.end(); ) {
        if((*it).second->VBOState != VBO_NONE){
            m_VBODeletionQueue.insert((*it).second);
        }
        it++;
    }

    m_chunksLastGen = currDrawChunks;

    updateGenerationThreads();
    updateVBOThreads();
    updateVBOGenQueue();

}
