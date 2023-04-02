#include "transform.h"

mat4 Transform::GetTransform()
{
    return translate(mat4(1.0f), pos) *
               rotate(mat4(1.0f), radians(rot.x), vec3(1.0f, 0.0f, 0.0f)) *
               rotate(mat4(1.0f), radians(rot.y), vec3(0.0f, 1.0f, 0.0f)) *
               rotate(mat4(1.0f), radians(rot.z), vec3(0.0f, 0.0f, 1.0f)) *
               scale(mat4(1.0f), scaleVec);
}

