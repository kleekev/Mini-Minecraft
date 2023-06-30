#pragma once
#include <QRunnable>
#include <QMutex>
#include "chunk.h"

class VBOWorker : public QRunnable
{
private:
    std::unordered_set<Chunk *> * m_VBOChunks;
    QMutex * m_VBOCompletedLock;
    Chunk * c;
public:
    VBOWorker(Chunk * c, std::unordered_set<Chunk *> * m_VBOChunks, QMutex * m_VBOCompletedLock);

    void run() override;
};

