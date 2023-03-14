#pragma once

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "texture.h"
#include "vertexLayout.h"
#include "mesh.h"
#include "model.h"

CLASS_PTR(Context)
class Context
{
public:
    static ContextUPtr Create();
    void Render();
    void RenderIMGUI();
    void ProcessInput(GLFWwindow *window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

private:
    Context() {}

    bool Init();
    ProgramUPtr m_program;
    ProgramUPtr m_simpleProgram;
    MeshUPtr m_box;
    ModelUPtr m_model;

private:
    // 창 크기
    int m_width{640};
    int m_height{480};

private:
    TextureUPtr m_texture;
    TextureUPtr m_texture2;

private:
    // camera parameter
    float m_cameraPitch{0.0f};
    float m_cameraYaw{0.0f};
    glm::vec3 m_cameraPos{glm::vec3(0.0f, 0.0f, 3.0f)};
    glm::vec3 m_cameraFront{glm::vec3(0.0f, 0.0f, -1.0f)};
    glm::vec3 m_cameraUp{glm::vec3(0.0f, 1.0f, 0.0f)};
    bool m_cameraControl{false};
    glm::vec2 m_prevMousePos{glm::vec2(0.0f)};

    std::vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f, 3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f, 2.0f, -2.5f),
        glm::vec3(1.5f, 0.2f, -1.5f),
        glm::vec3(-1.3f, 1.0f, -1.5f),
    };

private:
    // animation
    bool m_animation{true};

    // clear color
    glm::vec4 m_clearColor{glm::vec4(0.1f, 0.2f, 0.3f, 0.0f)};

    // light parameter
    struct Light
    {
        glm::vec3 position{glm::vec3(2.0f, 2.0f, 2.0f)};
        glm::vec3 direction{glm::vec3(-1.0f, -1.0f, -1.0f)};
        // inner cut-off angle, offset angle
        glm::vec2 cutoff{glm::vec2(20.0f, 5.0f)};
        float distance{32.0f};
        glm::vec3 ambient{glm::vec3(0.1f, 0.1f, 0.1f)};
        glm::vec3 diffuse{glm::vec3(0.5f, 0.5f, 0.5f)};
        glm::vec3 specular{glm::vec3(1.0f, 1.0f, 1.0f)};
    };
    Light m_light;
    bool m_freshLightMode{false}; // 손전등 모드

    MaterialPtr m_material;
};