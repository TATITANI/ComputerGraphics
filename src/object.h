#pragma once

#include "common.h"
#include "mesh.h"
using namespace glm;
using namespace std;

struct Camera;

CLASS_PTR(Transform)
class Transform
{
protected:
    vec3 pos;
    vec3 rot;
    vec3 scaleVec;

public:
    Transform(){};
    Transform(vec3 _pos, vec3 _rot, vec3 _scale)
        : pos(_pos), rot(_rot), scaleVec(_scale){};
    ~Transform(){};
    mat4 GetTransform();
};

CLASS_PTR(Object)
class Object
{
protected:
    MeshPtr mesh;
    ProgramPtr program;

    bool isInstance = false;
    vector<vec3> positions;
    BufferUPtr posBuffer;
    VertexLayoutUPtr instanceVAO;

public:
    Object(){};
    Object(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale)
        : mesh(_mesh), trf(_pos, _rot, _scale){};
    ~Object(){};
    Transform trf;

public:
    virtual void ActiveInstancing(size_t size, int atbIndex, int atbCount, int atbDivisor);
    virtual void Update(Camera &cam);
    virtual void Draw();
    virtual void AttatchProgram(ProgramPtr &_program, MaterialPtr mat = nullptr);

    virtual void Render(Camera &cam, ProgramPtr &_program, MaterialPtr mat = nullptr);
};

CLASS_PTR(Cubemap)
class Cubemap : public Object
{
public:
    Cubemap(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale)
        : Object(_mesh, _pos, _rot, _scale){};
    ~Cubemap(){};

    virtual void Update(Camera &cam) override;
};

CLASS_PTR(StencilBox)
class StencilBox
{
private:
    Object obj;
    MeshPtr mesh;
    ProgramPtr outlineProgram;
    mat4 trf;

    void Update(Camera &cam);
    void Draw(vec4 &color, float outlineSize);
    void AttatchProgram(ProgramPtr &_objPgm, ProgramPtr &_outlinePgm);

public:
    StencilBox(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale)
        : mesh(_mesh), obj(_mesh, _pos, _rot, _scale){};
    ~StencilBox(){};

    void Render(Camera &cam, ProgramPtr &_objPgm, ProgramPtr &_outlinePgm,
                vec4 &color, float outlineSize);
};
