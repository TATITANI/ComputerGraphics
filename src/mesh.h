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

    const VertexLayout *GetVertexLayout() const
    {
        return m_vertexLayout.get();
    }
    BufferPtr GetVertexBuffer() const { return m_vertexBuffer; }
    BufferPtr GetIndexBuffer() const { return m_indexBuffer; }

    void SetMaterial(MaterialPtr material) { m_material = material; }
    MaterialPtr GetMaterial() const { return m_material; }

    void Draw(const Program *program) const;

private:
    Mesh() {}
    void Init(
        const std::vector<Vertex> &vertices,
        const std::vector<uint32_t> &indices,
        uint32_t primitiveType);

    uint32_t m_primitiveType{GL_TRIANGLES};
    VertexLayoutUPtr m_vertexLayout; // VAO는 메시당 하나만 사용하므로 unique ptr
    BufferPtr m_vertexBuffer;        // vbo ibo는 vao에 연결하여 재사용할 수 있게 shared ptr
    BufferPtr m_indexBuffer;
    MaterialPtr m_material;
};
