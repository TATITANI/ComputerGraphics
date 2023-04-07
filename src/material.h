#pragma once

#include "common.h"
#include "program.h"
#include "texture.h"
#include <map>
#include <variant>
#include <vector>

using namespace std;
using namespace glm;

using FieldType = variant<monostate, int, float, vec2, vec3, vec4, mat4, TexturePtr>;

CLASS_PTR(Material)
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

CLASS_PTR(DeferredMaterial)
class DeferredMaterial : public Material
{
public:
    DeferredMaterial(const ProgramPtr &_program, const int &lightSize);
    ~DeferredMaterial() = default;
};

CLASS_PTR(SSAOMaterial)
class SSAOMaterial : public Material
{
public:
    SSAOMaterial(const ProgramPtr &_program);
    ~SSAOMaterial() = default;
};
