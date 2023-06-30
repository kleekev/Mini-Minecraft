#include "chunk.h"
#include <iostream>
#include <ostream>


Chunk::Chunk(OpenGLContext* mp_context, int minX, int minZ) : Drawable(mp_context),m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, minX(minX), minZ(minZ), genState(UNGENERATED), VBOState(VBO_NONE),
    VBOdirty(false), VBOready(false)
{
    std::fill_n(m_blocks.begin(), 65536, EMPTY);
}

Chunk::Chunk(OpenGLContext* mp_context, int minX, int minZ, GenState genState) : Drawable(mp_context),m_blocks(), m_neighbors{{XPOS, nullptr}, {XNEG, nullptr}, {ZPOS, nullptr}, {ZNEG, nullptr}}, minX(minX), minZ(minZ),
    genState(genState), VBOState(VBO_NONE)
{
}

// Does bounds checking with at()
BlockType Chunk::getBlockAt(unsigned int x, unsigned int y, unsigned int z) const {
    return m_blocks.at(x + 16 * y + 16 * 256 * z);
}

// Exists to get rid of compiler warnings about int -> unsigned int implicit conversion
BlockType Chunk::getBlockAt(int x, int y, int z) const {
    return getBlockAt(static_cast<unsigned int>(x), static_cast<unsigned int>(y), static_cast<unsigned int>(z));
}

// Does bounds checking with at()
void Chunk::setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t) {
    if(x + 16 * y + 16 * 256 * z > 65536){
        throw std::out_of_range("lol");
    }
    m_blocks.at(x + 16 * y + 16 * 256 * z) = t;
}


const static std::unordered_map<Direction, Direction, EnumHash> oppositeDirection {
    {XPOS, XNEG},
    {XNEG, XPOS},
    {YPOS, YNEG},
    {YNEG, YPOS},
    {ZPOS, ZNEG},
    {ZNEG, ZPOS}
};

void Chunk::linkNeighbor(uPtr<Chunk> &neighbor, Direction dir) {
    if(neighbor != nullptr) {
        this->m_neighbors[dir] = neighbor.get();
        neighbor->m_neighbors[oppositeDirection.at(dir)] = this;
        this->VBOdirty = true;
        neighbor->VBOdirty = true;

    }

}

void Chunk::setminX(int newX) {
    minX = newX;
}

void Chunk::setminZ(int newZ) {
    minZ = newZ;
}

void Chunk::setHumidity(float h) {
    humidity = h;
}

void Chunk::SendVBOdata(){

    std::vector<Vertex> &combinedVertexOpaque = this->VBOdata.combinedVertexOpaque;
    std::vector<GLuint> &combinedIdxOpaque = this->VBOdata.combinedIdxOpaque;
    std::vector<Vertex> &combinedVertexTransparrent = this->VBOdata.combinedVertexTransparrent;
    std::vector<GLuint> &combinedIdxtransparrent = this->VBOdata.combinedIdxTransparrent;

    m_countOpaque = combinedIdxOpaque.size();
    m_countTransparrent = combinedIdxtransparrent.size();
    generateIdxOpaque();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, combinedIdxOpaque.size() * sizeof(GLuint), combinedIdxOpaque.data(), GL_STATIC_DRAW);

    generateVertOpaque();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVertOpaque);
    mp_context->glBufferData(GL_ARRAY_BUFFER, combinedVertexOpaque.size() * sizeof(Vertex), combinedVertexOpaque.data(), GL_STATIC_DRAW);

    generateIdxTransparrent();
    mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransparrent);
    mp_context->glBufferData(GL_ELEMENT_ARRAY_BUFFER, combinedIdxtransparrent.size() * sizeof(GLuint), combinedIdxtransparrent.data(), GL_STATIC_DRAW);

    generateVertTransparent();
    mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVertTransparrent);
    mp_context->glBufferData(GL_ARRAY_BUFFER, combinedVertexTransparrent.size() * sizeof(Vertex), combinedVertexTransparrent.data(), GL_STATIC_DRAW);

    VBOState = VBO_DONE;
    VBOready = true;
    this->VBOdata.combinedVertexOpaque.clear();
    this->VBOdata.combinedIdxOpaque.clear();
    this->VBOdata.combinedVertexTransparrent.clear();
    this->VBOdata.combinedIdxTransparrent.clear();
}

void Chunk::deleteVBOdata(){
    if (VBOready != true){
        throw std::logic_error("VBO continuity error");
    }
    VBOState = VBO_NONE;
    VBOready = false;
    this->destroyVBOdata();
}

void Chunk::createVBOdata() {
    std::vector<GLuint> opaqueIdx, clearIdx, combinedIdxOpaque, combinedIdxtransparrent;
    std::vector<Vertex> opaqueData, clearData, combinedVertexOpaque, combinedVertexTransparrent;

        for (int x = 0;  x < 16; x++) {
            for (int y = 0; y < 256; y++) {
                for (int z = 0; z < 16; z++) {
                    BlockType t = getBlockAt(x, y, z);

                if (t != EMPTY) {
                    for (const BlockFace &f : adjacentFaces) {
                        BlockType adj;
                        bool crossed = false;
                        if (crossBorder(glm::ivec3(x,y,z), glm::ivec3(f.directionVec))) {
                            crossed = true;
                            Chunk* neighbor = m_neighbors[f.direction];
                            if (neighbor == nullptr) {
                                adj = EMPTY;
                            } else {
                                int dx = (x + (int)f.directionVec.x) % 16;
                                if (dx < 0) {
                                    dx = 16 + dx;
                                }
                                int dy = (y + (int)f.directionVec.y) % 256;
                                if (dy < 0) {
                                    dy = 256 + dy;
                                }
                                int dz = (z + (int)f.directionVec.z) % 16;
                                if (dz < 0) {
                                    dz = 16 + dz;
                                }
                                adj = neighbor->getBlockAt(dx, dy, dz);
                            }
                        } else {
                            adj = getBlockAt(x + (int)f.directionVec.x, y + (int)f.directionVec.y, z + (int)f.directionVec.z);
                        }
                        if (isClear(t)) {
                            if (adj == EMPTY && !crossed) {
                                appendVBOData(clearIdx, clearData, f, t, glm::ivec3(x,y,z), minX, minZ);
                            }
                        } else {
                            if (isClear(adj)) {
                                appendVBOData(opaqueIdx, opaqueData, f, t, glm::ivec3(x,y,z), minX, minZ);
                            }
                        }
                    }
                }
            }
        }
    }

    combineVBO(opaqueData,combinedVertexOpaque, combinedIdxOpaque);
    combineVBO(clearData,combinedVertexTransparrent, combinedIdxtransparrent);

    VBOdata.combinedVertexOpaque = std::move(combinedVertexOpaque);
    VBOdata.combinedIdxOpaque = std::move(combinedIdxOpaque);
    VBOdata.combinedVertexTransparrent = std::move(combinedVertexTransparrent);
    VBOdata.combinedIdxTransparrent = std::move(combinedIdxtransparrent);
}

void Chunk::appendVBOData(std::vector<GLuint> &idx, std::vector<Vertex> &data, const BlockFace &f, BlockType t, glm::ivec3 xyz, float world_x, float world_z) {
    int x = xyz.x, y = xyz.y, z = xyz.z;

    const std::array<VertexData, 4> &vertDat = f.vertices;

    glm::vec4 uvOffset = glm::vec4(blockUVs.at(t).at(f.direction), 0, 0);

    for (const VertexData &vd : vertDat) {
        // Pos
        glm::vec4 pos = glm::vec4(vd.pos.x + x + world_x, vd.pos.y + y, vd.pos.z + z + world_z, vd.pos.w);
        // Nor
        glm::vec4 nor = glm::vec4(f.directionVec.x, f.directionVec.y, f.directionVec.z, 0);
        // UV
        glm::vec4 uv = uvOffset + vd.uv;

        // Add humidity
        if (t == GRASS && f.direction == YPOS) {
            float xPosStep = glm::smoothstep(0.f, 15.f, 15.f-float(x));
            float xNegStep = glm::smoothstep(0.f, 15.f, float(x));
            float zPosStep = glm::smoothstep(0.f, 15.f, 15.f-float(z));
            float zNegStep = glm::smoothstep(0.f, 15.f, float(z));
            float xPosHumidity = glm::mix(m_neighbors[XPOS]->humidity, humidity, xPosStep);
            float xNegHumidity = glm::mix(m_neighbors[XNEG]->humidity, humidity, xNegStep);
            float zPosHumidity = glm::mix(m_neighbors[ZPOS]->humidity, humidity, zPosStep);
            float zNegHumidity = glm::mix(m_neighbors[ZNEG]->humidity, humidity, zNegStep);
            uv[2] = (xPosHumidity + xNegHumidity + zPosHumidity + zNegHumidity) / 4.f;
        }

        // Add flag to animated blocks
        if (isAnimated(t)) {
            uv[3] = 1;
        }

        // Vertex
        Vertex v = Vertex(pos, nor, uv);
        data.push_back(v);
    }
}

bool crossBorder(glm::ivec3 p1, glm::ivec3 p2) {
    glm::ivec3 p = p1 + p2;
    return (p.x < 0) || (p.x > 15) ||
           (p.y < 0) || (p.y > 255) ||
           (p.z < 0) || (p.z > 15) || false;

}

void Chunk::combineVBO(std::vector<Vertex> &data, std::vector<Vertex> &combinedVertex, std::vector<GLuint> &combinedIdx) {
    int maxIdx = 0;
    for (Vertex V : data) {
        combinedVertex.push_back(V);
        combinedIdx.push_back(0 + maxIdx);
        combinedIdx.push_back(1 + maxIdx);
        combinedIdx.push_back(2 + maxIdx);
        combinedIdx.push_back(0 + maxIdx);
        combinedIdx.push_back(2 + maxIdx);
        combinedIdx.push_back(3 + maxIdx);
        maxIdx += 4;
    }
}

int Chunk::getHeightAt(int x, int z){
    for (int i = 255; i>0; i--){
        if (getBlockAt(x,i,z) != EMPTY){
            return i;
        }
    }
    return -1;
}

bool isClear(BlockType t) {
    return clearBlocks.find(t) != clearBlocks.end();
}

bool isAnimated(BlockType t/*, Direction d*/) {
    return animatedBocks.find(t) != animatedBocks.end();
}
