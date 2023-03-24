#pragma once

#include "common.h"
#include "buffer.h"
#include "vertexlayout.h"
#include "texture.h"
#include "program.h"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

CLASS_PTR(Material);
class Material
{
public:
    static MaterialUPtr Create()
    {
        return MaterialUPtr(new Material());
    }
    TexturePtr diffuse;
    TexturePtr specular;
    float shininess{32.0f};
    void SetToProgram(const Program *program) const;

private:
    Material() {}
};

CLASS_PTR(Mesh);
class Mesh
{
public:
    static MeshUPtr Create(const std::vector<Vertex> &vertices,
                           const std::vector<uint32_t> &indices,
                           uint32_t primitiveType);

    static MeshUPtr Mesh::CreateBox();
    static MeshUPtr CreatePlane();
    static MeshUPtr MakeBox();

    const VertexLayout *GetVertexLayout()
        const { return m_vertexLayout.get(); }

    void BindVertexBuffer() { m_vertexBuffer->Bind(); }
    void BindIndexBuffer() { m_indexBuffer->Bind(); }

    void SetMaterial(MaterialPtr material) { m_material = material; }
    MaterialPtr GetMaterial() const { return m_material; }

    void Draw(const Program *program) const;
    void Draw(const Program *program, const VertexLayout *VAO, size_t instanceCnt) const;

private:
    Mesh() {}
    void Init(const std::vector<Vertex> &vertices,
              const std::vector<uint32_t> &indices,
              uint32_t primitiveType);

    uint32_t m_primitiveType{GL_TRIANGLES};
    VertexLayoutUPtr m_vertexLayout; // VAO
    BufferUPtr m_vertexBuffer;       // VBO
    BufferUPtr m_indexBuffer;        // IBO
    MaterialPtr m_material;
};
