#pragma once
#include "common.h"
#include "shader.h"

CLASS_PTR(Program)
class Program
{
public:
    static ProgramUPtr Create(
        const std::vector<ShaderPtr> &shaders);

    ~Program();
    uint32_t Get() const { return m_program; }
    void Use() const;

private:
    Program() {}
    bool Link(const std::vector<ShaderPtr> &shaders);
    uint32_t m_program{0};
};