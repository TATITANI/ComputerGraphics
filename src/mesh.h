#pragma once

#include "common.h"
#include "buffer.h"
#include "vertexlayout.h"
#include "texture.h"
#include "program.h"
#include <variant>
#include <tuple>
#include <map>
#include <vector>

using namespace std;

using FieldType = variant<monostate, int, float, glm::vec3, glm ::vec4, glm::mat4, TexturePtr>;

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;
};

CLASS_PTR(Material);
class Material
{
protected:
protected:
    ProgramPtr program;
    unordered_map<string, FieldType> propertyTable; // value : (type - value)

    void InitProperty(vector<string> propertyNames);
    virtual void ApplyTexture(string key, TexturePtr tex, int textureNum);

public:
    Material() = default;
    Material(const ProgramPtr &_program);
    ~Material() = default;
    void Apply();

    void SetProperty(string key, FieldType value);
};

CLASS_PTR(TextureMaterial)
class TextureMaterial : public Material
{
protected:
    virtual void ApplyTexture(string key, TexturePtr tex, int textureNum) override;

public:
    TextureMaterial(const ProgramPtr &_program);
    ~TextureMaterial() = default;
};

CLASS_PTR(CubemapMaterial)
class CubemapMaterial : public Material
{
public:
    CubemapMaterial(const ProgramPtr &_program);
    ~CubemapMaterial() = default;
};

CLASS_PTR(NormalMapMaterial)
class NormalMapMaterial : public Material
{
public:
    NormalMapMaterial(const ProgramPtr &_program);
    ~NormalMapMaterial() = default;
};

CLASS_PTR(Mesh);
class Mesh
{
public:
    static MeshUPtr Create(const vector<Vertex> &vertices,
                           const vector<uint32_t> &indices,
                           uint32_t primitiveType);

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
