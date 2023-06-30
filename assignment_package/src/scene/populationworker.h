#pragma once
#include "chunk.h"
#include <QRunnable>
#include <QMutex>


class populationworker : public QRunnable
{
private:
    Chunk * m_chunk;
    std::unordered_set<Chunk *>* m_populatedChunks;
    QMutex * m_chunkPopulationLock;
    //bool spawnLocation[64] = {false};

    //bool checkNeighbors(int x, int z);

    void placeTree(int x, int y, int z);

    void placeSnowTree(int x, int y, int z);
public:
    populationworker(Chunk * m_chunkToPopulate, std::unordered_set<Chunk *> * m_populatedChunks, QMutex * m_chunkPopulationLock);

    void run() override;
};
