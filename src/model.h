#pragma once

#include "common.h"
#include "mesh.h"
#include "object.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Transform;

CLASS_PTR(Model);
class Model
{
public:
    Model(const std::string &filename, const MaterialPtr &_mat, const Transform &&_trf);

    void Render(const mat4 &view, const mat4 &projection,
                const MaterialPtr &optionMat = nullptr);

private:
    Transform transform;
    MaterialPtr material;

private:
    Model() = default;
    bool LoadByAssimp(const std::string &filename);
    void ProcessMesh(aiMesh *mesh, const aiScene *scene);
    void ProcessNode(aiNode *node, const aiScene *scene);

    using MeshData = pair<MeshPtr, int>; // mesh, materialID
    std::vector<MeshData> meshDatas;
    std::vector<pair<TexturePtr, TexturePtr>> textures; // diffuse, specular
};
