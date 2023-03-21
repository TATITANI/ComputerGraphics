#pragma once
#include "common.h"
#include "shader.h"

// Program : 쉐이더 객체 단위
CLASS_PTR(Program)
class Program
{
public:
    static ProgramUPtr Create(
        const std::vector<ShaderPtr> &shaders);

    static ProgramUPtr Create(
        const std::string &vertShaderFilename,
        const std::string &fragShaderFilename);
    ~Program();

public:
    uint32_t Get() const { return m_program; }
    void Use() const;
    void SetUniform(const std::string &name, int value) const;
    void SetUniform(const std::string &name, const glm::mat4 &value) const;
    void SetUniform(const std::string &name, float value) const;
    void SetUniform(const std::string &name, const glm::vec2 &value) const;
    void SetUniform(const std::string &name, const glm::vec3 &value) const;
    void SetUniform(const std::string &name, const glm::vec4 &value) const;

private:
    Program() {}
    bool Link(const std::vector<ShaderPtr> &shaders);
    uint32_t m_program{0};
};