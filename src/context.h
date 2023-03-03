#pragma once

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertexLayout.h"


CLASS_PTR(Context)
class Context
{
public:
    static ContextUPtr Create();
    void Render();

private:
    Context() {}
    bool Init();
    ProgramUPtr m_program;
    BufferUPtr m_vertexBuffer;
    BufferUPtr m_indexBuffer;
    VertexLayoutUPtr m_vertexLayout;

    uint32_t m_vertexArrayObject;
};