#pragma once
#include "smartpointerhelp.h"
#include "glm_includes.h"
#include <array>
#include <unordered_map>
#include <cstddef>
#include "drawable.h"
#include "chunkhelper.h"



// One Chunk is a 16 x 256 x 16 section of the world,
// containing all the Minecraft blocks in that area.
// We divide the world into Chunks in order to make
// recomputing its VBO data faster by not having to
// render all the world at once, while also not having
// to render the world block by block.

// TODO have Chunk inherit from Drawable
class Chunk : public Drawable {
private:
    // All of the blocks contained within this Chunk
    std::array<BlockType, 65536> m_blocks;

    // This Chunk's four neighbors to the north, south, east, and west
    // The third input to this map just lets us use a Direction as
    // a key for this map.
    // These allow us to properly determine

    void appendVBOData(std::vector<GLuint> &idx, std::vector<Vertex> &data, const BlockFace &f, BlockType t, glm::ivec3 xyz, float world_x, float world_z/*, int &maxIdx*/);
    void bufferInterleavedData(std::vector<GLuint> &idx, std::vector<Vertex> &data, unsigned int max);
    void combineVBO(std::vector<Vertex> &data, std::vector<Vertex> &combinedVertex, std::vector<GLuint> &combinedIdx);

public:
    std::unordered_map<Direction, Chunk*, EnumHash> m_neighbors;
    struct {             // Structure declaration
      std::vector<Vertex> combinedVertexOpaque;         // Member (int variable)
      std::vector<GLuint> combinedIdxOpaque;   // Member (string variable)
      std::vector<Vertex> combinedVertexTransparrent;         // Member (int variable)
      std::vector<GLuint> combinedIdxTransparrent;   // Member (string variable)
    } VBOdata;       // Structure variable

    int minX;
    int minZ;
    float humidity;
    void SendVBOdata();
    void deleteVBOdata();
    Biome biome;

    int getHeightAt(int x, int z);
    Chunk(OpenGLContext*, int minX, int minZ);
    Chunk(OpenGLContext* mp_context, int minX, int minZ, GenState genState);
    virtual ~Chunk(){}
    GenState genState;
    VBOState VBOState;
    bool VBOdirty;
    bool VBOready;
    BlockType getBlockAt(unsigned int x, unsigned int y, unsigned int z) const;
    BlockType getBlockAt(int x, int y, int z) const;
    void setBlockAt(unsigned int x, unsigned int y, unsigned int z, BlockType t);
    void linkNeighbor(uPtr<Chunk>& neighbor, Direction dir);
    void createVBOdata() override;
    void setminX(int);
    void setminZ(int);
    void setHumidity(float);
};
