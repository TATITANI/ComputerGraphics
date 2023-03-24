#include "context.h"
#include "image.h"
#include <imgui.h>
#include "texture.h"

Context::Context()
{
}

Context::~Context()
{
}

ContextUPtr Context::Create()
{
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}

bool Context::Init()
{
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
    bool isSuccess = true;
    m_box = Mesh::CreateBox();
    m_plane = Mesh::CreatePlane();

    try
    {
        InitShader();
        InitMaterial();
        InitObject();
    }
    catch (std::string strError)
    {
        SPDLOG_INFO("error : {}", strError);
        isSuccess = false;
    }

    return isSuccess;
}

void Context::InitShader()
{
    // 단색 큐브
    m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
    m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
    // SPDLOG_INFO("program id: {}", m_program->Get());
    m_textureProgram = Program::Create("./shader/texture.vs", "./shader/texture.fs");
    // m_postProgram = Program::Create("./shader/texture.vs", "./shader/invert.fs");
    m_postProgram = Program::Create("./shader/texture.vs", "./shader/gamma.fs");

    m_skyboxProgram = Program::Create("./shader/skybox.vs", "./shader/skybox.fs");
    m_envMapProgram = Program::Create("./shader/env_map.vs", "./shader/env_map.fs");

    m_grassProgram = Program::Create("./shader/grass.vs", "./shader/grass.fs");
}

void Context::InitMaterial()
{
    // 단색 매터리얼 생성
    TexturePtr darkGrayTexture = Texture::CreateFromImage(ImagePtr(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f))));

    TexturePtr grayTexture = Texture::CreateFromImage(ImagePtr(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))));

    m_windowTexture = Texture::CreateFromImage(ImagePtr(
        Image::Load("./image/blending_transparent_window.png")));

    m_planeMaterial = Material::Create();
    m_planeMaterial->diffuse = Texture::CreateFromImage(ImagePtr(
        Image::Load("./image/marble.jpg")));
    m_planeMaterial->specular = grayTexture;
    m_planeMaterial->shininess = 128.0f;

    m_box1Material = Material::Create();
    m_box1Material->diffuse = Texture::CreateFromImage(ImagePtr(
        Image::Load("./image/container.jpg")));
    m_box1Material->specular = darkGrayTexture;
    m_box1Material->shininess = 16.0f;

    m_box2Material = Material::Create();
    m_box2Material->diffuse = Texture::CreateFromImage(ImagePtr(
        Image::Load("./image/container2.png")));
    m_box2Material->specular = Texture::CreateFromImage(
        ImagePtr(Image::Load("./image/container2_specular.png")));
    m_box2Material->shininess = 64.0f;

    // skybox
    auto cubeRight = Image::Load("./image/skybox/right.jpg", false);
    auto cubeLeft = Image::Load("./image/skybox/left.jpg", false);
    auto cubeTop = Image::Load("./image/skybox/top.jpg", false);
    auto cubeBottom = Image::Load("./image/skybox/bottom.jpg", false);
    auto cubeFront = Image::Load("./image/skybox/front.jpg", false);
    auto cubeBack = Image::Load("./image/skybox/back.jpg", false);
    m_skyboxTexture = CubeTexture::CreateFromImages({
        cubeRight.get(),
        cubeLeft.get(),
        cubeTop.get(),
        cubeBottom.get(),
        cubeFront.get(),
        cubeBack.get(),
    });

    m_grassTexture = Texture::CreateFromImage(Image::Load("./image/grass.png"));
  
}

void Context::InitObject()
{
    objSkybox = ObjectUPtr(new Object(m_box, m_camera.Pos, vec3(1), vec3(50)));
    objGround = ObjectUPtr(new Object(m_box, vec3(0.0f, -0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), glm::vec3(10.0f, 1.0f, 10.0f)));
    objBox1 = ObjectUPtr(new Object(m_box, vec3(-1.0f, 0.75f, -4.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.5f, 1.5f, 1.5f)));
    stencilBox = StencilBoxUPtr(new StencilBox(m_box, vec3(0.0f, 0.75f, 2.0f), vec3(0, 20, 0), vec3(1.5f)));
    objPlane1 = ObjectUPtr(new Object(m_plane, vec3(0, 0.5f, 4.0f), vec3(0), vec3(1)));
    objPlane2 = ObjectUPtr(new Object(m_plane, vec3(0.2f, 0.5f, 5.0f), vec3(0), vec3(1)));
    objPlane3 = ObjectUPtr(new Object(m_plane, vec3(0.4f, 0.5f, 6.0f), vec3(0), vec3(1)));
    objCubemap = CubemapUPtr(new Cubemap(m_box, vec3(1.0f, 0.75f, -2.0f), vec3(0, 40, 0), vec3(1.5f)));
    objGrass = ObjectUPtr(new Object(m_plane, vec3(0.0f, 0.5f, 0.0f), vec3(0), vec3(1)));
    objGrass->ActiveInstancing(10000, 3, 3, 1);
}

void Context::UpdateLight(mat4 &projection, mat4 &view)
{
    glm::vec3 lightPos = m_light.position;
    glm::vec3 lightDir = m_light.direction;
    if (m_freshLightMode)
    {
        lightPos = m_camera.Pos;
        lightDir = m_camera.Front;
    }
    else
    {
        // 광원
        auto lightModelTransform = glm::translate(glm::mat4(1.0), m_light.position) *
                                   glm::scale(glm::mat4(1.0), glm::vec3(0.1f));

        m_simpleProgram->Use();
        m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
        m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
        m_box->Draw(m_simpleProgram.get());
    }

    m_program->Use();
    m_program->SetUniform("viewPos", m_camera.Pos);
    m_program->SetUniform("light.position", lightPos);
    m_program->SetUniform("light.direction", lightDir);
    m_program->SetUniform("light.cutoff", glm::vec2(cosf(glm::radians(m_light.cutoff[0])),
                                                    cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_program->SetUniform("light.ambient", m_light.ambient);
    m_program->SetUniform("light.diffuse", m_light.diffuse);
    m_program->SetUniform("light.specular", m_light.specular);
}


void Context::UpdateCamera()
{
    m_camera.Front = glm::rotate(glm::mat4(1.0f), glm::radians(m_camera.Yaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
                     glm::rotate(glm::mat4(1.0f), glm::radians(m_camera.Pitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
                     glm::vec4(0.0f, 0.0f, -1.0f, 0.0f); // 4차원 성분이 0이면 벡터, 1이면 점

    // 종횡비 4:3, 세로화각 45도의 원근 투영
    m_camera.projection = glm::perspective(glm::radians(45.0f), (float)m_width / (float)m_height, 0.01f, 100.0f);
    m_camera.view = glm::lookAt(m_camera.Pos, m_camera.Pos + m_camera.Front, m_camera.Up);
}

void Context::Render()
{
    RenderIMGUI();

    m_framebuffer->Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    UpdateCamera();
    UpdateLight(m_camera.projection, m_camera.view);

    // skybox
    objSkybox->Render(m_camera, m_skyboxProgram);

    m_program->SetUniform("material.diffuse", 0); // texture slot
    m_program->SetUniform("material.specular", 1);

    objGround->Render(m_camera, m_program, m_planeMaterial);

    objBox1->Render(m_camera, m_program, m_box1Material);

    stencilBox->Render(m_camera, m_program, m_simpleProgram, glm::vec4(1.0f, 1.0f, 0.5f, 1.0f), 1.05f);

    // alpha blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    // glCullFace(GL_BACK); // 뒷면 컬링

    m_windowTexture->Bind();
    m_textureProgram->SetUniform("tex", 0);

    objPlane1->Render(m_camera, m_textureProgram);
    objPlane2->Render(m_camera, m_textureProgram);
    objPlane3->Render(m_camera, m_textureProgram);
    objCubemap->Render(m_camera, m_envMapProgram);

    m_grassProgram->SetUniform("tex", 0);
    m_grassTexture->Bind();
    objGrass->Render(m_camera, m_grassProgram);


    // post process
    Framebuffer::BindToDefault();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    m_postProgram->Use();
    m_postProgram->SetUniform("transform",
                              glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 1.0f)));
    m_framebuffer->GetColorAttachment()->Bind();
    m_postProgram->SetUniform("tex", 0);
    m_postProgram->SetUniform("gamma", m_gamma);
    m_plane->Draw(m_postProgram.get());
}

void Context::RenderIMGUI()
{
    if (ImGui::Begin("UI window")) // 펼쳤을 때
    {
        if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat3("l.position", glm::value_ptr(m_light.position), 0.01f);
            ImGui::DragFloat3("l.direction", glm::value_ptr(m_light.direction), 0.01f);
            ImGui::DragFloat("l.distance", &m_light.distance, 0.5f, 0, 3000.0f);
            ImGui::DragFloat2("l.cutoff", glm::value_ptr(m_light.cutoff), 0.5f, 0, 180.0f);
            ImGui::ColorEdit3("l.ambient", glm::value_ptr(m_light.ambient));
            ImGui::ColorEdit3("l.diffuse", glm::value_ptr(m_light.diffuse));
            ImGui::ColorEdit3("l.specular", glm::value_ptr(m_light.specular));
            ImGui::Checkbox("flash light", &m_freshLightMode);
        }

        ImGui::Checkbox("animation", &m_animation);

        if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor)))
        {
            glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        ImGui::DragFloat("gamma", &m_gamma, 0.01f, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_camera.Pos), 0.01f);
        ImGui::DragFloat("camera yaw", &m_camera.Yaw), 0.5f;
        ImGui::DragFloat("camera pitch", &m_camera.Pitch, 0.5f, -89, 89);
        ImGui::Separator();
        if (ImGui::Button("reset camera"))
        {
            m_camera.Yaw = 0;
            m_camera.Pitch = 0;
            m_camera.Pos = glm::vec3(0, 0, 3);
        }
        ImGui::Text("This is first text...");
    }

    float aspectRatio = (float)m_width / m_height;
    ImGui::Image((ImTextureID)m_framebuffer->GetColorAttachment()->Get(), ImVec2(150 * aspectRatio, 150));

    ImGui::End();
}

void Context::ProcessInput(GLFWwindow *window)
{
    if (!m_camera.Control)
        return;

    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_camera.Pos += cameraSpeed * m_camera.Front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_camera.Pos -= cameraSpeed * m_camera.Front;

    auto cameraRight = glm::normalize(glm::cross(m_camera.Up, -m_camera.Front));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera.Pos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera.Pos -= cameraSpeed * cameraRight;

    auto cameraUp = glm::normalize(glm::cross(-m_camera.Front, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_camera.Pos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_camera.Pos -= cameraSpeed * cameraUp;
}

void Context::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);

    m_framebuffer = Framebuffer::Create(Texture::Create(width, height, GL_RGBA));
}

void Context::MouseMove(double x, double y)
{
    if (!m_camera.Control)
        return;

    static glm::vec2 prevPos = glm::vec2((float)x, (float)y);
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - prevPos;

    const float cameraRotSpeed = 0.5f;
    m_camera.Yaw -= deltaPos.x * cameraRotSpeed;
    m_camera.Pitch -= deltaPos.y * cameraRotSpeed;

    if (m_camera.Yaw < 0.0f)
        m_camera.Yaw += 360.0f;
    if (m_camera.Yaw > 360.0f)
        m_camera.Yaw -= 360.0f;

    if (m_camera.Pitch > 89.0f)
        m_camera.Pitch = 89.0f;
    if (m_camera.Pitch < -89.0f)
        m_camera.Pitch = -89.0f;

    prevPos = pos;
}

void Context::MouseButton(int button, int action, double x, double y)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            // 마우스 조작 시작 시점에 현재 마우스 커서 위치 저장
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_camera.Control = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_camera.Control = false;
        }
    }
}
