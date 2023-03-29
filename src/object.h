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
private:
    friend class StencilBox;

protected:
    MeshPtr mesh;
    ProgramPtr defaultProgram;
    ProgramPtr currentProgram;

    bool isInstance = false;
    vector<vec3> positions;
    BufferUPtr posBuffer;
    VertexLayoutUPtr instanceVAO;

public:
    Object(){};
    Object(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, const ProgramPtr &pgm)
        : mesh(_mesh), trf(_pos, _rot, _scale), defaultProgram(pgm), currentProgram(pgm){};
    ~Object(){};
    Transform trf;

public:
    virtual void ActiveInstancing(size_t size, int atbIndex, int atbCount, int atbDivisor);
    virtual void Update(const Camera &cam);
    virtual void Update(const mat4 &view, const mat4 &projection);
    virtual void Draw();
    void UseProgram(const ProgramPtr &_program, const MaterialPtr &mat = nullptr);
    void UseProgram(const MaterialPtr &mat = nullptr);

    virtual void Render(const Camera &cam, const MaterialPtr &mat = nullptr,
                        const ProgramPtr &optionPgm = nullptr);
    virtual void Render(const mat4 &view, const mat4 &projection,
                        const MaterialPtr &mat = nullptr, const ProgramPtr &optionPgm = nullptr);
};

CLASS_PTR(Cubemap)
class Cubemap : public Object
{
public:
    Cubemap(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, const ProgramPtr &pgm)
        : Object(_mesh, _pos, _rot, _scale, pgm){};
    ~Cubemap(){};

    virtual void Render(const Camera &cam, const MaterialPtr &mat = nullptr,
                        const ProgramPtr &optionPgm = nullptr) override;
    virtual void Update(const Camera &cam) override;
};

CLASS_PTR(StencilBox)
class StencilBox
{
private:
    Object obj;
    MeshPtr mesh;
    ProgramPtr outlineProgram;
    mat4 trf;

    void Update(const mat4 &view, const mat4 &projection);
    void Draw(vec4 &color, float outlineSize);
    void UseProgram(const ProgramPtr &optionPgm, const ProgramPtr &_outlinePgm);

public:
    StencilBox(MeshPtr &_mesh, vec3 _pos, vec3 _rot, vec3 _scale, const ProgramPtr &pgm)
        : mesh(_mesh), obj(_mesh, _pos, _rot, _scale, pgm){};
    ~StencilBox(){};

    void Render(const Camera &cam, const ProgramPtr &optionPgm, ProgramPtr &_outlinePgm,
                vec4 &color, float outlineSize);

    void Render(const mat4 &view, const mat4 &projection, const ProgramPtr &optionPgm,
                ProgramPtr &_outlinePgm, vec4 &color, float outlineSize);
};
