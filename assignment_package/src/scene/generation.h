#pragma once
#include "noise.h"
#include "chunk.h"

namespace Generation
{
void GenerateChunk(Chunk* c, int xChunk, int zChunk);

bool CanPlaceTree(Chunk *c, int x, int y, int z);

bool CanPlaceCactus(Chunk *c, int x, int y, int z);

bool CanPlaceSnowTree(Chunk *c, int x, int y, int z);
}
