#include "mesh.h"

MeshUPtr Mesh::Create(const std::vector<Vertex> &vertices,
                      const std::vector<uint32_t> &indices, uint32_t primitiveType)
{
    auto mesh = MeshUPtr(new Mesh());
    mesh->Init(vertices, indices, primitiveType);
    return std::move(mesh);
}

void Mesh::ComputeTangents(std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
{
    // 역행렬 반환
    auto compute = [](const glm::vec3 &pos1, const glm::vec3 &pos2, const glm::vec3 &pos3,
                      const glm::vec2 &uv1, const glm::vec2 &uv2, const glm::vec2 &uv3)
        -> glm::vec3
    {
        auto edge1 = pos2 - pos1;
        auto edge2 = pos3 - pos1;
        auto deltaUV1 = uv2 - uv1;
        auto deltaUV2 = uv3 - uv1;
        float det = (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        if (det != 0.0f)
        {
            auto invDet = 1.0f / det;
            return invDet * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
        }
        else // 역행렬 없을 때
        {
            return glm::vec3(0.0f, 0.0f, 0.0f);
        }
    };

    // initialize
    std::vector<glm::vec3> tangents;
    tangents.resize(vertices.size());
    memset(tangents.data(), 0, tangents.size() * sizeof(glm::vec3));

    // accumulate triangle tangents to each vertex
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        auto v1 = indices[i];
        auto v2 = indices[i + 1];
        auto v3 = indices[i + 2];

        tangents[v1] += compute(
            vertices[v1].position, vertices[v2].position, vertices[v3].position,
            vertices[v1].texCoord, vertices[v2].texCoord, vertices[v3].texCoord);

        tangents[v2] = compute(
            vertices[v2].position, vertices[v3].position, vertices[v1].position,
            vertices[v2].texCoord, vertices[v3].texCoord, vertices[v1].texCoord);

        tangents[v3] = compute(
            vertices[v3].position, vertices[v1].position, vertices[v2].position,
            vertices[v3].texCoord, vertices[v1].texCoord, vertices[v2].texCoord);
    }

    // normalize
    for (size_t i = 0; i < vertices.size(); i++)
    {
        vertices[i].tangent = glm::normalize(tangents[i]);
    }
}

void Mesh::Init(const std::vector<Vertex> &vertices,
                const std::vector<uint32_t> &indices, uint32_t primitiveType)
{
    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.
    if (primitiveType == GL_TRIANGLES)
    {
        ComputeTangents(const_cast<std::vector<Vertex> &>(vertices), indices);
    }

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
    m_vertexLayout->SetAttrib(3, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, tangent));
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

