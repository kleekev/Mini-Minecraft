#pragma once
#include "chunk.h"
#include <QRunnable>
#include <QMutex>

class BlockTypeWorker : public QRunnable
{
private:
    int m_xCorner, m_zCorner;
    std::vector<Chunk *> m_chunksToGenerate;
    std::unordered_set<Chunk *>* m_generatedChunks;
    QMutex * m_chunkCompletedLock;
public:
    BlockTypeWorker(int m_xCorner, int m_zCorner, std::vector<Chunk *> m_chunksToGenerate, std::unordered_set<Chunk *>* m_generatedChunks, QMutex * m_chunkCompletedLock);

    void run() override;
};
