#pragma once
#include "chunk.h"
#include <array>
#include <unordered_set>

//using namespace std;

// C++ 11 allows us to define the size of an enum. This lets us use only one byte
// of memory to store our different block types. By default, the size of a C++ enum
// is that of an int (so, usually four bytes). This *does* limit us to only 256 different
// block types, but in the scope of this project we'll never get anywhere near that many.
#define BLK_UVX * 0.0625f
#define BLK_UVY * 0.0625f
#define BLK_UV 0.0625f
enum BlockType : unsigned char
{
    EMPTY, GRASS, DIRT, STONE, WATER, SNOW, LAVA, BEDROCK, OAK_LOG, OAK_LEAVES, SAND, SANDSTONE, CACTUS, ICE
};

enum GenState : unsigned char
{
    UNGENERATED, TERRAIN_RUNNING, TERRAIN_DONE, POPULATION_RUNNING, GEN_COMPLETE
};

enum VBOState : unsigned char
{
    VBO_NONE, VBO_WAITING, VBO_RUNNING, VBO_DONE,
};

enum Biome : unsigned char
{
    GRASSLAND, MOUNTAIN, DESERT, SNOWLAND,
};

// The six cardinal directions in 3D space
enum Direction : unsigned char
{
    XPOS, XNEG, YPOS, YNEG, ZPOS, ZNEG
};

// Lets us use any enum class as the key of a
// std::unordered_map
struct EnumHash {
    template <typename T>
    size_t operator()(T t) const {
        return static_cast<size_t>(t);
    }
};

struct VertexData {
    glm::vec4 pos;
    glm::vec4 uv;

    VertexData(glm::vec4 p, glm::vec4 u)
        : pos(p), uv(u)
    {}
};

struct BlockFace {
    Direction direction;
    glm::vec3 directionVec;
    std::array<VertexData, 4> vertices;
    BlockFace(Direction dir, glm::vec3 dV, const VertexData &a, const VertexData &b, const VertexData &c, const VertexData &d)
        : direction(dir), directionVec(dV), vertices{a, b, c, d}
    {}
};

struct Vertex {
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec4 uv;

    Vertex(glm::vec4 p, glm::vec4 n, glm::vec4 u)
        :   pos(p), nor(n), uv(u)
    {}
};

const static std::array<BlockFace, 6> adjacentFaces {
    BlockFace(XPOS, glm::vec3(1, 0, 0), VertexData(glm::vec4(1,0,1,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(1,0,0,1), glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(1,1,0,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(1,1,1,1), glm::vec4(0,BLK_UV,0,0))),

    BlockFace(XNEG, glm::vec3(-1, 0, 0),VertexData(glm::vec4(0,0,0,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(0,0,1,1), glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(0,1,1,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(0,1,0,1), glm::vec4(0,BLK_UV,0,0))),

    BlockFace(YPOS, glm::vec3(0, 1, 0), VertexData(glm::vec4(0,1,1,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(1,1,1,1),  glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(1,1,0,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(0,1,0,1), glm::vec4(0,BLK_UV,0,0))),

    BlockFace(YNEG, glm::vec3(0, -1, 0),VertexData(glm::vec4(0,0,0,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(1,0,0,1),  glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(1,0,1,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(0,0,1,1), glm::vec4(0,BLK_UV,0,0))),

    BlockFace(ZPOS, glm::vec3(0, 0, 1), VertexData(glm::vec4(0,0,1,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(1,0,1,1),  glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(1,1,1,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(0,1,1,1),glm::vec4(0,BLK_UV,0,0))),

    BlockFace(ZNEG, glm::vec3(0, 0, -1),VertexData(glm::vec4(1,0,0,1), glm::vec4(0,0,0,0)),
                                        VertexData(glm::vec4(0,0,0,1),  glm::vec4(BLK_UV,0,0,0)),
                                        VertexData(glm::vec4(0,1,0,1), glm::vec4(BLK_UV,BLK_UV,0,0)),
                                        VertexData(glm::vec4(1,1,0,1), glm::vec4(0,BLK_UV,0,0)))

};

const static std::unordered_map<BlockType, std::unordered_map<Direction, glm::vec2, EnumHash>, EnumHash> blockUVs
{
    {DIRT, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {XPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)}}},
    {STONE, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                {XPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                {ZNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                {ZPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                {YNEG, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)},
                                                                {YPOS, glm::vec2(1.f BLK_UVX, 15.f BLK_UVY)}}},
    {SNOW, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(4.f BLK_UVX, 11.f BLK_UVY)},
                                                               {XPOS, glm::vec2(4.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZNEG, glm::vec2(4.f BLK_UVX, 11.f BLK_UVY)},
                                                               {ZPOS, glm::vec2(4.f BLK_UVX, 11.f BLK_UVY)},
                                                               {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                               {YPOS, glm::vec2(2.f BLK_UVX, 11.f BLK_UVY)}}},
    {WATER, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)},
                                                                {XPOS, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)},
                                                                {ZNEG, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)},
                                                                {ZPOS, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)},
                                                                {YNEG, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)},
                                                                {YPOS, glm::vec2(13.f BLK_UVX, 3.f BLK_UVY)}}},
    {GRASS, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                {XPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                {ZNEG, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                {ZPOS, glm::vec2(3.f BLK_UVX, 15.f BLK_UVY)},
                                                                {YNEG, glm::vec2(2.f BLK_UVX, 15.f BLK_UVY)},
                                                                {YPOS, glm::vec2(8.f BLK_UVX, 13.f BLK_UVY)}}},
    {LAVA, std::unordered_map<Direction, glm::vec2, EnumHash>  {{XNEG, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)},
                                                                {XPOS, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)},
                                                                {ZNEG, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)},
                                                                {ZPOS, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)},
                                                                {YNEG, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)},
                                                                {YPOS, glm::vec2(13.f BLK_UVX, 1.f BLK_UVY)}}},
    {BEDROCK, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(1.f BLK_UVX, 14.f BLK_UVY)}}},
    {OAK_LOG, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(4.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 14.f BLK_UVY)}}},
    {OAK_LEAVES, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 12.f BLK_UVY)}}},
    {SAND, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {XPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {ZPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YNEG, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)},
                                                              {YPOS, glm::vec2(2.f BLK_UVX, 14.f BLK_UVY)}}},
    {SANDSTONE, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(0.f BLK_UVX, 3.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(0.f BLK_UVX, 2.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(0.f BLK_UVX, 4.f BLK_UVY)}}},
    {CACTUS, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(6.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(7.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(5.f BLK_UVX, 11.f BLK_UVY)}}},
    {ICE, std::unordered_map<Direction, glm::vec2, EnumHash> {{XNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {XPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {ZPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YNEG, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)},
                                                                  {YPOS, glm::vec2(3.f BLK_UVX, 11.f BLK_UVY)}}}
};

const static std::unordered_map<BlockType, glm::vec3, EnumHash> blockColor {
    {GRASS, glm::vec3(95.f, 159.f, 53.f) / 255.f},
    {DIRT, glm::vec3(121.f, 85.f, 58.f) / 255.f},
    {STONE, glm::vec3(0.5f)},
    {WATER, glm::vec3(0.f, 0.f, 0.75f)},
    {SNOW, glm::vec3(1.f, 1.f, 1.0f)},
    {LAVA,  glm::vec3(1.f, 0.f, 0.f)},
    {BEDROCK, glm::vec3(0.f, 0.f, 0.f)}
};

const static std::unordered_set<BlockType, EnumHash> clearBlocks
{
    EMPTY, WATER, LAVA, ICE
};

const static std::unordered_set<BlockType, EnumHash> animatedBocks
{
    WATER, LAVA
};

bool crossBorder(glm::ivec3 p1, glm::ivec3 p2);
bool isClear(BlockType);
bool isAnimated(BlockType);
