#include "drawable.h"
#include <glm_includes.h>

Drawable::Drawable(OpenGLContext* context)
    : m_countOpaque(-1), m_bufIdxOpaque(), m_bufIdxTransparrent(), m_bufPos(), m_bufNor(), m_bufCol(), m_bufVertOpaque(), m_bufVertTransparrent(),
      m_idxOpaqueGenerated(false), m_idxTransparrentGenerated(false), m_posGenerated(false), m_norGenerated(false), m_colGenerated(false), m_vertOpaqueGenerated(false), m_vertTransparrentGenerated(false),
      m_uvGenerated(false),
      mp_context(context)
{

}

Drawable::~Drawable()
{}


void Drawable::destroyVBOdata()
{
    mp_context->glDeleteBuffers(1, &m_bufIdxOpaque);
    mp_context->glDeleteBuffers(1, &m_bufIdxTransparrent);
    mp_context->glDeleteBuffers(1, &m_bufPos);
    mp_context->glDeleteBuffers(1, &m_bufNor);
    mp_context->glDeleteBuffers(1, &m_bufCol);
    mp_context->glDeleteBuffers(1, &m_bufVertOpaque);
    mp_context->glDeleteBuffers(1, &m_bufVertTransparrent);
    m_idxOpaqueGenerated = m_idxTransparrentGenerated = m_posGenerated = m_norGenerated = m_colGenerated = m_vertOpaqueGenerated = m_vertTransparrentGenerated =  m_uvGenerated =false;
    m_countOpaque = -1;
    m_countTransparrent = -1;

}

GLenum Drawable::drawMode()
{
    // Since we want every three indices in bufIdx to be
    // read to draw our Drawable, we tell that the draw mode
    // of this Drawable is GL_TRIANGLES

    // If we wanted to draw a wireframe, we would return GL_LINES

    return GL_TRIANGLES;
}

int Drawable::elemCountOpaque()
{
    return m_countOpaque;
}

int Drawable::elemCountTransparrent()
{
    return m_countTransparrent;
}

void Drawable::generateIdxOpaque()
{
    if (!m_idxOpaqueGenerated){
        m_idxOpaqueGenerated = true;
        // Create a VBO on our GPU and store its handle in bufIdx
        mp_context->glGenBuffers(1, &m_bufIdxOpaque);
    }


}

void Drawable::generateIdxTransparrent()
{
    if (!m_idxTransparrentGenerated){
        m_idxTransparrentGenerated = true;
        // Create a VBO on our GPU and store its handle in bufIdx
        mp_context->glGenBuffers(1, &m_bufIdxTransparrent);
    }
}

void Drawable::generatePos()
{
    if (!m_posGenerated){
        m_posGenerated = true;
        // Create a VBO on our GPU and store its handle in bufPos
        mp_context->glGenBuffers(1, &m_bufPos);
    }
}

void Drawable::generateNor()
{
    if (!m_norGenerated){
        m_norGenerated = true;
        // Create a VBO on our GPU and store its handle in bufNor
        mp_context->glGenBuffers(1, &m_bufNor);
    }

}

void Drawable::generateCol()
{
    if (!m_colGenerated){
        m_colGenerated = true;
        // Create a VBO on our GPU and store its handle in bufCol
        mp_context->glGenBuffers(1, &m_bufCol);
    }

}

void Drawable::generateVertOpaque()
{
    if (!m_vertOpaqueGenerated){
        m_vertOpaqueGenerated = true;
        // Create a VBO on our GPU and store its handle in m_bufVert
        mp_context->glGenBuffers(1, &m_bufVertOpaque);
    }

}

void Drawable::generateVertTransparent()
{
    if (!m_vertTransparrentGenerated){
        m_vertTransparrentGenerated = true;
        // Create a VBO on our GPU and store its handle in m_bufVert
        mp_context->glGenBuffers(1, &m_bufVertTransparrent);
    }

}

void Drawable::generateUV()
{
    if (!m_uvGenerated){
        m_uvGenerated = true;
        // Create a VBO on our GPU and store its handle in m_bufUV
        mp_context->glGenBuffers(1, &m_bufUV);
    }

}

bool Drawable::bindIdxOpaque()
{
    if(m_idxOpaqueGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxOpaque);
    }
    return m_idxOpaqueGenerated;
}

bool Drawable::bindIdxTransparrent()
{
    if(m_idxTransparrentGenerated) {
        mp_context->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bufIdxTransparrent);
    }
    return m_idxTransparrentGenerated;
}

bool Drawable::bindPos()
{
    if(m_posGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPos);
    }
    return m_posGenerated;
}

bool Drawable::bindNor()
{
    if(m_norGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufNor);
    }
    return m_norGenerated;
}

bool Drawable::bindCol()
{
    if(m_colGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufCol);
    }
    return m_colGenerated;
}

bool Drawable::bindVertTransparent()
{
    if(m_vertTransparrentGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVertTransparrent);
    }
    return m_vertTransparrentGenerated;
}

bool Drawable::bindVertOpaque()
{
    if(m_vertOpaqueGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufVertOpaque);
    }
    return m_vertOpaqueGenerated;
}

bool Drawable::bindUV()
{
    if(m_uvGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufUV);
    }
    return m_uvGenerated;
}

InstancedDrawable::InstancedDrawable(OpenGLContext *context)
    : Drawable(context), m_numInstances(0), m_bufPosOffset(-1), m_offsetGenerated(false)
{}

InstancedDrawable::~InstancedDrawable(){}

int InstancedDrawable::instanceCount() const {
    return m_numInstances;
}

void InstancedDrawable::generateOffsetBuf() {
    m_offsetGenerated = true;
    mp_context->glGenBuffers(1, &m_bufPosOffset);
}

bool InstancedDrawable::bindOffsetBuf() {
    if(m_offsetGenerated){
        mp_context->glBindBuffer(GL_ARRAY_BUFFER, m_bufPosOffset);
    }
    return m_offsetGenerated;
}

void InstancedDrawable::clearOffsetBuf() {
    if(m_offsetGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufPosOffset);
        m_offsetGenerated = false;
    }
}
void InstancedDrawable::clearColorBuf() {
    if(m_colGenerated) {
        mp_context->glDeleteBuffers(1, &m_bufCol);
        m_colGenerated = false;
    }
}
