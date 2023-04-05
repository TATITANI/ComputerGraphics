#pragma once

#include "common.h"
#include "material.h"
#include "buffer.h"
#include "program.h"
#include "vertexlayout.h"
#include "texture.h"
#include <vector>
#include <variant>
#include <map>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
    glm::vec3 tangent;
};

CLASS_PTR(Mesh)
class Mesh
{
public:
    static MeshUPtr Create(const vector<Vertex> &vertices,
                           const vector<uint32_t> &indices,
                           uint32_t primitiveType);
    static void ComputeTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);
    static MeshUPtr Mesh::CreateBox();
    static MeshUPtr CreatePlane();
    static MeshUPtr MakeBox();

    const VertexLayout *GetVertexLayout()
        const { return m_vertexLayout.get(); }

    void BindVertexBuffer() { m_vertexBuffer->Bind(); }
    void BindIndexBuffer() { m_indexBuffer->Bind(); }

    void Draw() const;
    void Draw(const VertexLayout *VAO, size_t instanceCnt) const;

private:
    Mesh() {}
    void Init(const vector<Vertex> &vertices,
              const vector<uint32_t> &indices,
              uint32_t primitiveType);

    uint32_t m_primitiveType{GL_TRIANGLES};
    VertexLayoutUPtr m_vertexLayout; // VAO
    BufferUPtr m_vertexBuffer;       // VBO
    BufferUPtr m_indexBuffer;        // IBO
    MaterialPtr m_material;
};
