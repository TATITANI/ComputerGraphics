#pragma once

#include "common.h"
using namespace glm;

class Transform
{

public:
    Transform(){};
    Transform(vec3 _pos, vec3 _rot, vec3 _scale)
        : pos(_pos), rot(_rot), scaleVec(_scale){};
    ~Transform(){};
    vec3 pos;
    vec3 rot;
    vec3 scaleVec;
    mat4 GetTransform()    {
        return translate(mat4(1.0f), pos) *
               rotate(mat4(1.0f), radians(rot.x), vec3(1.0f, 0.0f, 0.0f)) *
               rotate(mat4(1.0f), radians(rot.y), vec3(0.0f, 1.0f, 0.0f)) *
               rotate(mat4(1.0f), radians(rot.z), vec3(0.0f, 0.0f, 1.0f)) *
               scale(mat4(1.0f), scaleVec);
    };
};
