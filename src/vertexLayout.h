#pragma once

#include "common.h"

CLASS_PTR(VertexLayout)
class VertexLayout
{
public:
    static VertexLayoutUPtr Create();
    ~VertexLayout();

    uint32_t Get() const { return m_vertexArrayObject; }
    void SetAttrib(
        uint32_t attribIndex, int count,
        uint32_t type, bool normalized,
        size_t stride, uint64_t offset) const;

    void DisableAttrib(int attribIndex) const;
    void Bind() const;

private:
    VertexLayout() {}
    void Init();
    uint32_t m_vertexArrayObject{0};
};