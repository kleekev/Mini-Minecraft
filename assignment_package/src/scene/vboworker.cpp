#include "vboworker.h"

VBOWorker::VBOWorker(Chunk * c, std::unordered_set<Chunk *> * m_VBOChunks, QMutex * m_VBOCompletedLock):
    m_VBOChunks(m_VBOChunks), m_VBOCompletedLock(m_VBOCompletedLock), c(c)
{

}
void VBOWorker::run() {
    c->createVBOdata();
    m_VBOCompletedLock->lock();
    m_VBOChunks->insert(c);
    m_VBOCompletedLock->unlock();
}
