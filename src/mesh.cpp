#include "mesh.h"

MeshUPtr Mesh::Create(const std::vector<Vertex> &vertices,
                      const std::vector<uint32_t> &indices, uint32_t primitiveType)
{
    auto mesh = MeshUPtr(new Mesh());
    mesh->Init(vertices, indices, primitiveType);
    return std::move(mesh);
}

void Mesh::Init(const std::vector<Vertex> &vertices,
                const std::vector<uint32_t> &indices, uint32_t primitiveType)
{
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    m_vertexLayout = VertexLayout::Create();
    // VBO
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                                            vertices.data(), sizeof(Vertex), vertices.size());
    // 인덱스 버퍼
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW,
                                           indices.data(), sizeof(uint32_t), indices.size());
    // VAO
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, false, sizeof(Vertex), 0);
    m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, normal));
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, texCoord));
}

void Mesh::Draw() const
{
    m_vertexLayout->Bind();
    glDrawElements(m_primitiveType, m_indexBuffer->GetCount(), GL_UNSIGNED_INT, 0);
}

void Mesh::Draw(const VertexLayout *VAO, size_t instanceCnt) const
{
    VAO->Bind();
    glDrawElementsInstanced(GL_TRIANGLES, m_indexBuffer->GetCount(),
                            GL_UNSIGNED_INT, 0, instanceCnt);
}

MeshUPtr Mesh::CreateBox()
{
    // pos.xyz, normal.xyz, texcoord.uv
    std::vector<Vertex> vertices = {
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)},

        Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},

        Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

        Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

        Vertex{glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},

        Vertex{glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
    };

    std::vector<uint32_t> indices = {
        0, 2, 1, 2, 0, 3,       //
        4, 5, 6, 6, 7, 4,       //
        8, 9, 10, 10, 11, 8,    //
        12, 14, 13, 14, 12, 15, //
        16, 17, 18, 18, 19, 16, //
        20, 22, 21, 22, 20, 23, //
    };

    return Create(vertices, indices, GL_TRIANGLES);
}

MeshUPtr Mesh::CreatePlane()
{
    std::vector<Vertex> vertices = {
        Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)},
        Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},
        Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},
    };

    std::vector<uint32_t> indices = {
        0,
        1,
        2,
        2,
        3,
        0,
    };

    return Create(vertices, indices, GL_TRIANGLES);
}

void Material::InitProperty(vector<string> propertyNames)
{
    for (auto name : propertyNames)
    {
        FieldType f;
        propertyTable[name] = f;
    }
}

void Material::ApplyTexture(string key, TexturePtr tex, int textureNum)
{
    glActiveTexture(GL_TEXTURE0 + textureNum);
    program->SetUniform(key, textureNum);
    tex->Bind();
}

Material::Material(const ProgramPtr &_program)
{
    program = _program;

    InitProperty({"transform", "modelTransform","color",
                  "material.diffuse", "material.specular", "material.shininess"});
}

void Material::Apply()
{
    program->Use();

    int textureNum = 0;

    for (auto property : propertyTable)
    {
        auto value = property.second;
        string key = property.first;
        visit(overloaded{
                  [&](auto p)
                  { program->SetUniform(key, p); },
                  [&](TexturePtr p)
                  {
                      ApplyTexture(key, p, textureNum);
                      textureNum++;
                  },
                  [&](monostate) {},
              },
              value);
    }
}

void Material::SetProperty(string key, FieldType value)
{
    if (propertyTable.count(key) == 0)
    {
        SPDLOG_INFO("material key does not exist : {}", key);
        return;
    }
    propertyTable[key] = value;
}

void TextureMaterial::ApplyTexture(string key, TexturePtr tex, int textureNum)
{
    // alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_BACK); // 뒷면 컬링
    glActiveTexture(GL_TEXTURE0 + textureNum);
    program->SetUniform(key, textureNum);
    tex->Bind();
}

TextureMaterial::TextureMaterial(const ProgramPtr &_program)
{
    program = _program;
    InitProperty({"transform", "modelTransform", "tex"});
}

NormalMapMaterial::NormalMapMaterial(const ProgramPtr &_program)
{
    program = _program;
    InitProperty({"transform", "modelTransform", "viewPos",
                  "lightPos", "diffuse", "normalMap"});
}

CubemapMaterial::CubemapMaterial(const ProgramPtr &_program)
{
    program = _program;
    InitProperty({"model", "view", "projection",
                  "cameraPos", "skybox"});
}
