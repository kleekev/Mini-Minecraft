#include "blocktypeworker.h"
#include "generation.h"

BlockTypeWorker::BlockTypeWorker(int m_xCorner, int m_zCorner, std::vector<Chunk *> m_chunksToGenerate,
                                 std::unordered_set<Chunk *>* m_generatedChunks, QMutex * m_chunkCompletedLock)
    : m_xCorner(m_xCorner), m_zCorner(m_zCorner), m_chunksToGenerate(m_chunksToGenerate), m_generatedChunks(m_generatedChunks),
      m_chunkCompletedLock(m_chunkCompletedLock){}


void BlockTypeWorker::run() {
    for (Chunk * c:  m_chunksToGenerate){
        Generation::GenerateChunk(c, c->minX, c->minZ);
        m_chunkCompletedLock->lock();
        m_generatedChunks->insert(c);
        m_chunkCompletedLock->unlock();
    }
}
