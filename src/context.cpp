#include "context.h"
#include "image.h"
#include <imgui.h>

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

    m_model = Model::Load("./model/backpack.obj");
    if (!m_model)
        return false;

    m_box = Mesh::CreateBox();
    // 단색 큐브
    m_simpleProgram = Program::Create("./shader/simple.vs", "./shader/simple.fs");
    if (!m_simpleProgram)
        return false;

    m_program = Program::Create("./shader/lighting.vs", "./shader/lighting.fs");
    if (!m_program)
        return false;
    SPDLOG_INFO("program id: {}", m_program->Get());

    ////// texture
    auto image = ImagePtr(Image::Load("./image/container.jpg"));
    if (!image)
        return false;
    SPDLOG_INFO("image: {}x{}, {} channels", image->GetWidth(), image->GetHeight(), image->GetChannelCount());
    m_texture = Texture::CreateFromImage(image);

    auto image2 = ImagePtr(Image::Load("./image/awesomeface.png"));
    m_texture2 = Texture::CreateFromImage(image2);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture->Get());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_texture2->Get());

    m_program->Use(); // don't forget to activate the shader before setting uniforms!
    // uniform 변수 설정
    m_program->SetUniform("tex", 0);
    m_program->SetUniform("tex2", 1);

    // x축으로 -55도 회전
    auto model = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 카메라는 원점으로부터 z축 방향으로 -3만큼 떨어짐
    auto view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    // 종횡비 4:3, 세로화각 45도의 원근 투영
    auto projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 10.0f);
    auto transform = projection * view * model;
    m_program->SetUniform("transform", transform);

    m_material = Material::Create();
    m_material->diffuse = Texture::CreateFromImage(ImagePtr(
        Image::CreateSingleColorImage(4, 4, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f))));
    m_material->specular = Texture::CreateFromImage(ImagePtr(
        Image::CreateSingleColorImage(4, 4, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f))));

    return true;
}

void Context::Render()
{
    RenderIMGUI();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_cameraFront =
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f); // 4차원 성분이 0이면 벡터, 1이면 점

    auto projection = glm::perspective(glm::radians(45.0f), (float)m_width / (float)m_height, 0.01f, 20.0f);

    const float radius = 10.0f;
    float camX = sin(glfwGetTime()) * radius;
    float camZ = cos(glfwGetTime()) * radius;
    auto cameraPos = glm::vec3(camX, 0.0f, camZ);
    auto cameraTarget = glm::vec3(0, 0.0f, 0.0f);
    auto cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    auto view = glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);

    glm::vec3 lightPos = m_light.position;
    glm::vec3 lightDir = m_light.direction;
    if (m_freshLightMode)
    {
        lightPos = m_cameraPos;
        lightDir = m_cameraFront;
    }
    else
    {
        // 광원
        auto lightModelTransform =
            glm::translate(glm::mat4(1.0), m_light.position) *
            glm::scale(glm::mat4(1.0), glm::vec3(0.1f));

        m_simpleProgram->Use();
        m_simpleProgram->SetUniform("color", glm::vec4(m_light.ambient + m_light.diffuse, 1.0f));
        m_simpleProgram->SetUniform("transform", projection * view * lightModelTransform);
        m_box->Draw(m_simpleProgram.get());
    }

    m_program->Use();
    m_program->SetUniform("viewPos", m_cameraPos);
    m_program->SetUniform("light.position", lightPos);
    m_program->SetUniform("light.direction", lightDir);
    m_program->SetUniform("light.cutoff", glm::vec2(
                                              cosf(glm::radians(m_light.cutoff[0])),
                                              cosf(glm::radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_program->SetUniform("light.ambient", m_light.ambient);
    m_program->SetUniform("light.diffuse", m_light.diffuse);
    m_program->SetUniform("light.specular", m_light.specular);

    m_program->SetUniform("material.diffuse", 0); // texture slot
    m_program->SetUniform("material.specular", 1);
    m_program->SetUniform("material.shininess", m_material->shininess);

    glActiveTexture(GL_TEXTURE0);
    m_material->diffuse->Bind();
    glActiveTexture(GL_TEXTURE1);
    m_material->specular->Bind();

    auto modelTransform = glm::mat4(1.0f);
    auto transform = projection * view * modelTransform;
    m_program->SetUniform("transform", transform);
    m_program->SetUniform("modelTransform", modelTransform);
    m_model->Draw(m_program.get());

    // for (size_t i = 0; i < cubePositions.size(); i++)
    // {
    //     auto &pos = cubePositions[i];
    //     auto angle = glm::radians((m_animation ? (float)glfwGetTime() : 0) * 120.0f + 20.0f * (float)i);
    //     auto modelTrf = glm::translate(glm::mat4(1.0f), pos);
    //     modelTrf = glm::rotate(modelTrf, angle, glm::vec3(1.0f, 0.5f, 0.0f));

    //     auto transform = projection * view * modelTrf;
    //     m_program->SetUniform("transform", transform);
    //     m_program->SetUniform("modelTransform", modelTrf);
    //     // primitive 타입, ebo내 index 개수, index 데이터형, ebo 첫 데이터의 오프셋
    //     m_box->Draw();
    // }
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

        if (ImGui::CollapsingHeader("material", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::DragFloat("m.shininess", &m_material->shininess, 1.0f, 1.0f, 256.0f);
        }

        ImGui::Checkbox("animation", &m_animation);

        if (ImGui::ColorEdit4("clear color", glm::value_ptr(m_clearColor)))
        {
            glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        ImGui::Separator();
        ImGui::DragFloat3("camera pos", glm::value_ptr(m_cameraPos), 0.01f);
        ImGui::DragFloat("camera pos", &m_cameraYaw), 0.5f;
        ImGui::DragFloat("camera pitch", &m_cameraPitch, 0.5f, -89, 89);
        ImGui::Separator();
        if (ImGui::Button("reset camera"))
        {
            m_cameraYaw = 0;
            m_cameraPitch = 0;
            m_cameraPos = glm::vec3(0, 0, 3);
        }
        ImGui::Text("This is first text...");
    }
    ImGui::End();
}

void Context::ProcessInput(GLFWwindow *window)
{
    if (!m_cameraControl)
        return;

    const float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}

void Context::Reshape(int width, int height)
{
    m_width = width;
    m_height = height;
    glViewport(0, 0, m_width, m_height);
}

void Context::MouseMove(double x, double y)
{
    if (!m_cameraControl)
        return;

    static glm::vec2 prevPos = glm::vec2((float)x, (float)y);
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - prevPos;

    const float cameraRotSpeed = 0.5f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)
        m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f)
        m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)
        m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f)
        m_cameraPitch = -89.0f;

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
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_cameraControl = false;
        }
    }
}
