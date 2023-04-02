#include "model.h"
#include "transform.h"

Model::Model(const std::string &filename, const MaterialPtr &_mat, const Transform &&_trf)
{
    if (LoadByAssimp(filename))
    {
        material = _mat;
        transform = _trf;
    }
}

bool Model::LoadByAssimp(const std::string &filename)
{
    Assimp::Importer importer;
    auto scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        SPDLOG_ERROR("failed to load model: {}", filename);
        return false;
    }

    auto dirname = filename.substr(0, filename.find_last_of("/"));
    auto LoadTexture = [&](aiMaterial *aiMaterial, aiTextureType type) -> TexturePtr
    {
        if (aiMaterial->GetTextureCount(type) <= 0)
            return nullptr;
        aiString filepath;
        aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &filepath);
        auto image = Image::Load(fmt::format("{}/{}", dirname, filepath.C_Str()));
        if (!image)
            return nullptr;
        return Texture::CreateFromImage(std::move(image));
    };

    for (uint32_t i = 0; i < scene->mNumMaterials; i++)
    {
        auto aiMaterial = scene->mMaterials[i];

        auto diffuseTex = LoadTexture(aiMaterial, aiTextureType_DIFFUSE);
        auto specTex = LoadTexture(aiMaterial, aiTextureType_SPECULAR);

        textures.push_back({diffuseTex, specTex});
    }

    ProcessNode(scene->mRootNode, scene);
    return true;
}

void Model::ProcessNode(aiNode *node, const aiScene *scene)
{
    for (uint32_t i = 0; i < node->mNumMeshes; i++)
    {
        auto meshIndex = node->mMeshes[i];
        auto mesh = scene->mMeshes[meshIndex];
        ProcessMesh(mesh, scene);
    }

    for (uint32_t i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene);
    }
}

void Model::ProcessMesh(aiMesh *mesh, const aiScene *scene)
{
    SPDLOG_INFO("process mesh: {}, #vert: {}, #face: {}",
                mesh->mName.C_Str(), mesh->mNumVertices, mesh->mNumFaces);

    std::vector<Vertex> vertices;
    vertices.resize(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        auto &v = vertices[i];
        v.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        v.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        v.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }

    std::vector<uint32_t> indices;
    indices.resize(mesh->mNumFaces * 3);
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        indices[3 * i] = mesh->mFaces[i].mIndices[0];
        indices[3 * i + 1] = mesh->mFaces[i].mIndices[1];
        indices[3 * i + 2] = mesh->mFaces[i].mIndices[2];
    }

    auto glMesh = Mesh::Create(vertices, indices, GL_TRIANGLES);
    meshDatas.push_back({std::move(glMesh), mesh->mMaterialIndex});
}

void Model::Render(const mat4 &view, const mat4 &projection, const MaterialPtr &optionMat)
{
    auto modelTransform = this->transform.GetTransform(); // world
    auto transform = projection * view * modelTransform;  // clip space

    for (auto &data : meshDatas)
    {
        auto mesh = data.first;
        int materialID = data.second;
        if (materialID >= 0)
        {
            MaterialPtr mat = optionMat ? optionMat : material;
            mat->SetProperty("material.diffuse", textures[materialID].first);
            mat->SetProperty("material.specular", textures[materialID].second);
            mat->SetProperty("transform", transform);
            mat->SetProperty("modelTransform", modelTransform);
            mat->Apply();
        }

        mesh->Draw();
    }
}
