#pragma once

#include "common.h"
#include "shader.h"
#include "program.h"
#include "buffer.h"
#include "vertexLayout.h"
#include "material.h"
#include "mesh.h"
#include "model.h"
#include "framebuffer.h"
#include "object.h"
#include "shadowmap.h"

using namespace glm;
using namespace std;

struct Camera
{
public:
    mat4 projection;
    mat4 view;
    vec3 Pos{vec3(0.0f, 2.0f, 12.0f)};
    float Pitch{0.0f};
    float Yaw{0.0f};
    vec3 Front{vec3(0.0f, 0.0f, -1.0f)};
    vec3 Up{vec3(0.0f, 1.0f, 0.0f)};
    bool Control{false};
};

struct Light
{
    bool directional{false};
    vec3 position{vec3(2.0f, 4.0f, 4.0f)};
    vec3 direction{vec3(-0.5f, -1.5f, -1.0f)};
    // inner cut-off angle, offset angle
    vec2 cutoff{vec2(50.0f, 5.0f)};
    float distance{150.0f};
    vec3 ambient{vec3(0.8f, 0.8f, 0.8f)};
    vec3 diffuse{vec3(0.5f, 0.5f, 0.5f)};
    vec3 specular{vec3(1.0f, 1.0f, 1.0f)};
};

struct DeferLight
{
    glm::vec3 position;
    glm::vec3 color;
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
    void InitParameters();

    void UpdateLight(mat4 &projection, mat4 &view);
    void UpdateCamera();
    void DrawShadowedObjects(const mat4 &view, const mat4 &projection,
                             const MaterialPtr &optionMat = nullptr);
    void DrawShadowedObjects(const Camera &cam,
                             const MaterialPtr &optionMat = nullptr);

    void GenerateShadowMap();
    void RenderDeffered();

private:
    ProgramPtr m_program;
    ProgramPtr m_simpleProgram;
    ProgramPtr m_textureProgram;
    ProgramPtr m_lightingShadowProgram;
    ProgramPtr m_normalProgram;

    MeshPtr m_box;
    MeshPtr m_plane;

    // framebuffer
    FramebufferUPtr m_framebuffer;
    ProgramUPtr m_postProgram;
    // cubemap
    CubeTextureUPtr m_skyboxTexture;
    ProgramPtr m_skyboxProgram;
    ProgramPtr m_envMapProgram;

    // grass
    ProgramPtr m_grassProgram;

    // shadow map
    ShadowMapUPtr m_shadowMap;

    // deferred shading
    FramebufferPtr m_deferGeoFramebuffer;

    ProgramPtr m_deferGeoProgram;
    ProgramPtr m_deferLightProgram;

private:
    MaterialPtr m_skyboxMaterial;
    MaterialPtr m_groundMaterial;
    MaterialPtr m_box1Material;
    MaterialPtr m_box2Material;
    MaterialPtr modelMaterial;
    MaterialPtr shadowmapMaterial;
    NormalMapMaterialPtr m_wallMaterial;
    TextureMaterialPtr m_grassMaterial;
    CubemapMaterialPtr m_cubeMapMaterial;
    TextureMaterialPtr m_planeMaterial;

    MaterialPtr deferredGeoBoxMaterial;
    MaterialPtr deferredGeoGroundMaterial;
    MaterialPtr deferredLightMaterial;

    vector<DeferLight> m_deferLights;

private:
    ObjectUPtr objSkybox;
    ObjectUPtr objGround;
    ObjectUPtr objBox1;
    StencilBoxUPtr stencilBox;
    ObjectUPtr objPlane1;
    ObjectUPtr objPlane2;
    ObjectUPtr objPlane3;
    CubemapUPtr objCubemap;
    ObjectUPtr objGrass;
    WallUPtr objWall;
    ModelUPtr m_model;
    DeferredPlanePtr objDeferredPlane;
    ObjectUPtr objDeferredGround;
    ObjectUPtr objDeferredBox;


private:
    // 창 크기
    int m_width{640};
    int m_height{480};

private:
    Camera m_camera;

private:
    vec2 m_prevMousePos{vec2(0.0f)};

    // animation
    bool m_animation{true};

    // clear color
    vec4 m_clearColor{vec4(0.1f, 0.2f, 0.3f, 0.0f)};

    // light parameter
    Light m_light;
    bool m_freshLightMode{false}; // 손전등 모드
    bool m_blinn{true};

private:
    float m_gamma{1.0f};
};