#pragma once

#include "transform.h"
#include "common.h"
#include "mesh.h"
using namespace glm;
using namespace std;

struct Camera;

CLASS_PTR(Object)
class Object
{
private:
    friend class StencilBox;

protected:
    MeshPtr mesh;
    MaterialPtr material; // default
    MaterialPtr optionMaterial;
    MaterialPtr currentMaterial;

    bool isInstance = false;
    vector<vec3> positions;
    BufferUPtr posBuffer;
    VertexLayoutUPtr instanceVAO;
    void SetCurrentMaterial(const MaterialPtr &mat){currentMaterial = mat;};
public:
    Object(){};
    Object(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, MaterialPtr _mat)
        : mesh(_mesh), trf(_pos, _rot, _scale), material(_mat), currentMaterial(_mat){};
    Object(MeshPtr &_mesh, vec3 _pos)
        : mesh(_mesh), trf(_pos, vec3(0), vec3(1)){};

    ~Object(){};
    Transform trf;

public:
    virtual void ActiveInstancing(size_t size, int atbIndex, int atbCount, int atbDivisor);
    virtual void Update(const Camera &cam);
    virtual void Update(const mat4 &view, const mat4 &projection);
    virtual void Draw();

    virtual void Render(const Camera &cam, const MaterialPtr &optionMat = nullptr);
    virtual void Render(const mat4 &view, const mat4 &projection,
                        const MaterialPtr &optionMat = nullptr);
};

CLASS_PTR(Cubemap)
class Cubemap : public Object
{
public:
    Cubemap(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, MaterialPtr _mat)
        : Object(_mesh, _pos, _rot, _scale, _mat){};
    ~Cubemap(){};

    virtual void Render(const Camera &cam, const MaterialPtr &optionMat = nullptr) override;
    virtual void Update(const Camera &cam) override;
};

CLASS_PTR(Wall)
class Wall : public Object
{
private:
    void Update(const Camera &cam, const vec3 &lightPos);

public:
    Wall(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, MaterialPtr _mat)
        : Object(_mesh, _pos, _rot, _scale, _mat){};
    ~Wall(){};

    void Render(const Camera &cam, const vec3 &lightPos, const MaterialPtr &optionMat = nullptr);
};

CLASS_PTR(StencilBox)
class StencilBox
{
private:
    Object obj;
    MeshPtr mesh;
    ProgramPtr outlinePgm;
    mat4 trf;

    void Update(const mat4 &view, const mat4 &projection);
    void Draw(vec4 &color, float outlineSize);

public:
    StencilBox(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, MaterialPtr _mat)
        : mesh(_mesh), obj(_mesh, _pos, _rot, _scale, _mat){};
    ~StencilBox(){};

    void Render(const Camera &cam, const MaterialPtr &optionMat, ProgramPtr &_outlinPgmt,
                vec4 &color, float outlineSize);

    void Render(const mat4 &view, const mat4 &projection, const MaterialPtr &optionMat,
                ProgramPtr &_outlineMat, vec4 &color, float outlineSPgm);
};
