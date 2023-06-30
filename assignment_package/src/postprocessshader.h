#pragma once

#include "shaderprogram.h"

class PostProcessShader : public ShaderProgram
{
public:
    PostProcessShader(OpenGLContext* context);
    virtual ~PostProcessShader();

    // Draw the given object to our screen using this ShaderProgram's shaders
    void draw(Drawable &d, int textureSlot);
};
