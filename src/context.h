#pragma once

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertexLayout.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "object.h"

using namespace glm;
using namespace std;

struct Camera
{
public:
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 Pos{glm::vec3(0.0f, 2.0f, 12.0f)};
    float Pitch{0.0f};
    float Yaw{0.0f};
    glm::vec3 Front{glm::vec3(0.0f, 0.0f, -1.0f)};
    glm::vec3 Up{glm::vec3(0.0f, 1.0f, 0.0f)};
    bool Control{false};
};

CLASS_PTR(Context)
class Context
{
public:
    Context();
    ~Context();

    static ContextUPtr Create();
    void Render();
    void RenderIMGUI();
    void ProcessInput(GLFWwindow *window);
    void Reshape(int width, int height);
    void MouseMove(double x, double y);
    void MouseButton(int button, int action, double x, double y);

private:
    bool Init();
    void InitShader();
    void InitMaterial();
    void InitObject();

    void UpdateLight(mat4 &projection, mat4 &view);
    void UpdateCamera();

private:
    ProgramPtr m_program;
    ProgramPtr m_simpleProgram;
    ProgramPtr m_textureProgram;

    MeshPtr m_box;
    MeshPtr m_plane;
    TexturePtr m_windowTexture;
    // framebuffer
    FramebufferUPtr m_framebuffer;
    ProgramUPtr m_postProgram;
    // cubemap
    CubeTextureUPtr m_skyboxTexture;
    ProgramPtr m_skyboxProgram;
    ProgramPtr m_envMapProgram;

    // grass
    TexturePtr m_grassTexture;
    ProgramPtr m_grassProgram;

private:
    // 창 크기
    int m_width{640};
    int m_height{480};

private:
    Camera m_camera;

private:
    glm::vec2 m_prevMousePos{glm::vec2(0.0f)};

    // animation
    bool m_animation{true};

    // clear color
    glm::vec4 m_clearColor{glm::vec4(0.1f, 0.2f, 0.3f, 0.0f)};

    // light parameter
    struct Light
    {
        glm::vec3 position{glm::vec3(1.0f, 4.0f, 4.0f)};
        glm::vec3 direction{glm::vec3(-1.0f, -1.0f, -1.0f)};
        // inner cut-off angle, offset angle
        glm::vec2 cutoff{glm::vec2(120.0f, 5.0f)};
        float distance{120.0f};
        glm::vec3 ambient{glm::vec3(0.1f, 0.1f, 0.1f)};
        glm::vec3 diffuse{glm::vec3(0.5f, 0.5f, 0.5f)};
        glm::vec3 specular{glm::vec3(1.0f, 1.0f, 1.0f)};
    };
    Light m_light;
    bool m_freshLightMode{false}; // 손전등 모드
    bool m_blinn{false};


private:
    float m_gamma{1.0f};

private:
    MaterialPtr m_planeMaterial;
    MaterialPtr m_box1Material;
    MaterialPtr m_box2Material;

public:
    ObjectUPtr objSkybox;
    ObjectUPtr objGround;
    ObjectUPtr objBox1;
    StencilBoxUPtr stencilBox;
    ObjectUPtr objPlane1;
    ObjectUPtr objPlane2;
    ObjectUPtr objPlane3;
    CubemapUPtr objCubemap;
    ObjectUPtr objGrass;
};