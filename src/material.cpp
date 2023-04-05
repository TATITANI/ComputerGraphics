#include "material.h"

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

    InitProperty({"transform", "modelTransform", "color",
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

DeferredMaterial::DeferredMaterial(const ProgramPtr &_program, const int &lightSize)
{
    program = _program;
    vector<string> p = {"transform", "modelTransform", "gPosition", "gNormal", "gAlbedoSpec"};
    for (size_t i = 0; i < lightSize; i++)
    {
        p.push_back(fmt::format("lights[{}].position", i));
        p.push_back(fmt::format("lights[{}].color", i));
    }

    InitProperty(p);
}
