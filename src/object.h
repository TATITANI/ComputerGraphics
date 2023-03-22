#pragma once

#include "common.h"
#include "mesh.h"
#include "context.h"
using namespace glm;

struct Camera;

CLASS_PTR(Transform)
class Transform
{
protected:
    vec3 pos;
    vec3 rot;
    vec3 scaleVec;

public:
    Transform(vec3 _pos, vec3 _rot, vec3 _scale)
        : pos(_pos), rot(_rot), scaleVec(_scale){};
    ~Transform(){};
    mat4 GetTransform();
};

CLASS_PTR(Object)
class Object
{
protected:
    ProgramPtr program;

public:
    Object(vec3 _pos, vec3 _rot, vec3 _scale)
        : trf(_pos, _rot, _scale){};
    ~Object(){};
    Transform trf;

    virtual void Update(Camera &cam);
    virtual void Draw(MeshUPtr &mesh);
    virtual void AttatchProgram(ProgramPtr &_program, MaterialPtr mat = nullptr);

    virtual void Render(Camera &cam, MeshUPtr &mesh,
                        ProgramPtr &_program, MaterialPtr mat = nullptr);
};

CLASS_PTR(Cubemap)
class Cubemap : public Object
{
public:
    Cubemap(vec3 _pos, vec3 _rot, vec3 _scale)
        : Object(_pos, _rot, _scale){};
    ~Cubemap(){};

    virtual void Update(Camera &cam) override;
};

CLASS_PTR(StencilBox)
class StencilBox
{
private:
    Object obj;
    ProgramPtr outlineProgram;
    mat4 localTrf;

    void Update(Camera &cam);
    void Draw(MeshUPtr &mesh, vec4 &color, float outlineSize);
    void AttatchProgram(ProgramPtr &_objPgm, ProgramPtr &_outlinePgm);

public:
    StencilBox(vec3 _pos, vec3 _rot, vec3 _scale)
        : obj(_pos, _rot, _scale){};
    ~StencilBox(){};

    void Render(Camera &cam, ProgramPtr &_objPgm, ProgramPtr &_outlinePgm,
                MeshUPtr &mesh, vec4 &color, float outlineSize);
};
