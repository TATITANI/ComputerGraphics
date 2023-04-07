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
    glEnable(GL_MULTISAMPLE);
    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);
    bool isSuccess = true;
    m_box = Mesh::CreateBox();
    m_plane = Mesh::CreatePlane();

    try
    {
        InitParameters();
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
    m_lightingShadowProgram = Program::Create("./shader/lighting_shadow.vs", "./shader/lighting_shadow.fs");
    m_normalProgram = Program::Create("./shader/normal.vs", "./shader/normal.fs");

    m_deferGeoProgram = Program::Create("./shader/defer_geo.vs", "./shader/defer_geo.fs");
    m_deferLightProgram = Program::Create("./shader/defer_light.vs", "./shader/defer_light.fs");

    m_ssaoProgram = Program::Create("./shader/ssao.vs", "./shader/ssao.fs");
    m_blurProgram = Program::Create("./shader/blur_5x5.vs", "./shader/blur_5x5.fs");
}

void Context::InitMaterial()
{
    // 단색 매터리얼 생성
    TexturePtr darkGrayTexture = Texture::CreateFromImage(ImagePtr(Image::CreateSingleColorImage(4, 4, vec4(0.2f, 0.2f, 0.2f, 1.0f))));

    TexturePtr grayTexture = Texture::CreateFromImage(ImagePtr(Image::CreateSingleColorImage(4, 4, vec4(0.5f, 0.5f, 0.5f, 1.0f))));

    TexturePtr groundTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/marble.jpg")));
    TexturePtr boxTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/container.jpg")));
    TexturePtr box2Texture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/container2.png")));
    TexturePtr box2SpecTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/container2_specular.png")));
    TexturePtr wallTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/brickwall.jpg")));
    TexturePtr wallNormalTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/brickwall_normal.jpg")));
    TexturePtr planeTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/blending_transparent_window.png")));
    TexturePtr grassTexture = Texture::CreateFromImage(ImagePtr(Image::Load("./image/grass.png")));

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

    m_skyboxMaterial = MaterialPtr(new Material(m_skyboxProgram));

    m_groundMaterial = MaterialPtr(new Material(m_lightingShadowProgram));
    m_groundMaterial->SetProperty("material.diffuse", groundTexture);
    m_groundMaterial->SetProperty("material.specular", grayTexture);
    m_groundMaterial->SetProperty("material.shininess", 4.0f);

    m_box1Material = MaterialPtr(new Material(m_lightingShadowProgram));
    m_box1Material->SetProperty("material.diffuse", boxTexture);
    m_box1Material->SetProperty("material.specular", darkGrayTexture);
    m_box1Material->SetProperty("material.shininess", 16.0f);

    m_box2Material = MaterialPtr(new Material(m_lightingShadowProgram));
    m_box2Material->SetProperty("material.diffuse", box2Texture);
    m_box2Material->SetProperty("material.specular", box2SpecTexture);
    m_box2Material->SetProperty("material.shininess", 64.0f);

    modelMaterial = MaterialPtr(new Material(m_deferGeoProgram /*m_lightingShadowProgram*/));
    modelMaterial->SetProperty("material.shininess", 16.0f);

    m_wallMaterial = NormalMapMaterialPtr(new NormalMapMaterial(m_normalProgram));
    m_wallMaterial->SetProperty("diffuse", wallTexture);
    m_wallMaterial->SetProperty("normalMap", wallNormalTexture);

    m_planeMaterial = TextureMaterialPtr(new TextureMaterial(m_textureProgram));
    m_planeMaterial->SetProperty("tex", planeTexture);

    m_grassMaterial = TextureMaterialPtr(new TextureMaterial(m_grassProgram));
    m_grassMaterial->SetProperty("tex", grassTexture);

    m_cubeMapMaterial = CubemapMaterialPtr(new CubemapMaterial(m_envMapProgram));

    shadowmapMaterial = MaterialPtr(new Material(m_simpleProgram));
    shadowmapMaterial->SetProperty("color", vec4(1.0f, 1.0f, 1.0f, 1.0f));

    deferredGeoGroundMaterial = MaterialPtr(new Material(m_deferGeoProgram));
    deferredGeoGroundMaterial->SetProperty("material.diffuse", groundTexture);
    deferredGeoGroundMaterial->SetProperty("material.specular", darkGrayTexture);

    deferredGeoBoxMaterial = MaterialPtr(new Material(m_deferGeoProgram));
    deferredGeoBoxMaterial->SetProperty("material.diffuse", boxTexture);
    deferredGeoBoxMaterial->SetProperty("material.specular", darkGrayTexture);

    deferredLightMaterial = DeferredMaterialPtr(new DeferredMaterial(m_deferLightProgram, m_deferLights.size()));

    ssaoMaterial = SSAOMaterialPtr(new SSAOMaterial(m_ssaoProgram));
    ssaoBlurMaterial = TextureMaterialPtr(new TextureMaterial(m_blurProgram));

    std::vector<glm::vec3> ssaoNoise;
    ssaoNoise.resize(16);
    for (size_t i = 0; i < ssaoNoise.size(); i++)
    {
        // randonly selected tangent direction
        glm::vec3 sample(RandomRange(-1.0f, 1.0f),
                         RandomRange(-1.0f, 1.0f), 0.0f);
        ssaoNoise[i] = sample;
    }
    m_ssaoNoiseTexture = Texture::Create(4, 4, GL_RGB16F, GL_FLOAT);
    m_ssaoNoiseTexture->SetFilter(GL_NEAREST, GL_NEAREST);
    m_ssaoNoiseTexture->SetWrap(GL_REPEAT, GL_REPEAT);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, ssaoNoise.data());

    m_shadowMap = ShadowMap::Create(1024, 1024);
}

void Context::InitObject()
{
    objSkybox = ObjectUPtr(new Object(m_box, m_camera.Pos, vec3(1), vec3(50), m_skyboxMaterial));
    objGround = ObjectUPtr(new Object(m_box, vec3(0.0f, -0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(15.0f, 1.0f, 15.0f), m_groundMaterial));
    objBox1 = ObjectUPtr(new Object(m_box, vec3(-1.0f, 0.75f, -4.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.5f, 1.5f, 1.5f), m_box1Material));
    stencilBox = StencilBoxUPtr(new StencilBox(m_box, vec3(0.0f, 0.75f, 2.0f), vec3(0, 20, 0), vec3(1.5f), m_box2Material));
    objPlane1 = ObjectUPtr(new Object(m_plane, vec3(0, 0.5f, 4.0f), vec3(0), vec3(1), m_planeMaterial));
    objPlane2 = ObjectUPtr(new Object(m_plane, vec3(0.2f, 0.5f, 5.0f), vec3(0), vec3(1), m_planeMaterial));
    objPlane3 = ObjectUPtr(new Object(m_plane, vec3(0.4f, 0.5f, 6.0f), vec3(0), vec3(1), m_planeMaterial));
    objCubemap = CubemapUPtr(new Cubemap(m_box, vec3(1.0f, 0.75f, -2.0f), vec3(0, 40, 0), vec3(1.5f), m_cubeMapMaterial));
    objGrass = ObjectUPtr(new Object(m_plane, vec3(0.0f, 0.5f, 0.0f), vec3(0), vec3(1), m_grassMaterial));
    objGrass->ActiveInstancing(10000, 3, 3, 1);

    objWall = WallUPtr(new Wall(m_plane, vec3(0.0f, 3.0f, 0.0f), vec3(-45, 0, 0), vec3(1), m_wallMaterial));
    objDeferredPlane = DeferredPlanePtr(new DeferredPlane(m_plane, Transform(vec3(0), vec3(0), vec3(2)), deferredLightMaterial));
    objDeferredGround = ObjectUPtr(new Object(m_box, vec3(-20.0f, -0.5f, 0.0f), vec3(1.0f, 1.0f, 1.0f), vec3(15.0f, 1.0f, 15.0f), deferredGeoGroundMaterial));
    objDeferredBox = ObjectUPtr(new Object(m_box, vec3(-20.0f, 0.75f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(1.5f, 1.5f, 1.5f), deferredGeoBoxMaterial));
    objSSAOPlane = SSAOPlaneUPtr(new SSAOPlane(m_plane, Transform(vec3(0), vec3(0), vec3(2.f)), ssaoMaterial));
    objBlurPlane = BlurPlaneUPtr(new BlurPlane(m_plane, Transform(vec3(0), vec3(0), vec3(2.f)), ssaoBlurMaterial));

    m_model = ModelUPtr(new Model("./model/backpack.obj", modelMaterial, Transform(vec3(-20.f, 0.5f, 3.0f), vec3(-90, 0, 0), vec3(0.5f))));
}

void Context::InitParameters()
{
    m_deferLights.resize(32);
    for (size_t i = 0; i < m_deferLights.size(); i++)
    {
        m_deferLights[i].position = glm::vec3(
            RandomRange(-25.0f, -10.0f),
            RandomRange(1.0f, 3.0f),
            RandomRange(-5.0f, 5.0f));
        m_deferLights[i].color = glm::vec3(
            RandomRange(0.0f, i < 3 ? 1.0f : 0.0f),
            RandomRange(0.0f, i < 3 ? 1.0f : 0.0f),
            RandomRange(0.0f, i < 3 ? 1.0f : 0.0f));
    }

    m_ssaoSamples.resize(64);
    for (size_t i = 0; i < m_ssaoSamples.size(); i++)
    {
        // uniformly randomized point in unit hemisphere
        glm::vec3 sample(RandomRange(-1.0f, 1.0f),
                         RandomRange(-1.0f, 1.0f),
                         RandomRange(0.0f, 1.0f));
        sample = glm::normalize(sample) * RandomRange();

        // scale for slightly shift to center
        float t = (float)i / (float)m_ssaoSamples.size();
        float t2 = t * t;
        float scale = (1.0f - t2) * 0.1f + t2 * 1.0f;

        m_ssaoSamples[i] = sample * scale;
    }
}

void Context::UpdateLight(mat4 &projection, mat4 &view)
{
    vec3 lightPos = m_light.position;
    vec3 lightDir = m_light.direction;
    if (m_freshLightMode)
    {
        lightPos = m_camera.Pos;
        lightDir = m_camera.Front;
    }
    else
    {
        // 광원
        m_simpleProgram->Use();
        for (size_t i = 0; i < m_deferLights.size(); i++)
        {
            m_simpleProgram->SetUniform("color", glm::vec4(m_deferLights[i].color, 1.0f));
            m_simpleProgram->SetUniform("transform", projection * view *
                                                         glm::translate(glm::mat4(1.0f), m_deferLights[i].position) *
                                                         glm::scale(glm::mat4(1.0f), glm::vec3(0.1f)));
            m_box->Draw();
        }
    }

    m_program->Use();
    m_program->SetUniform("viewPos", m_camera.Pos);
    m_program->SetUniform("light.position", lightPos);
    m_program->SetUniform("light.direction", lightDir);
    m_program->SetUniform("light.cutoff", vec2(cosf(radians(m_light.cutoff[0])),
                                               cosf(radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_program->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_program->SetUniform("light.ambient", m_light.ambient);
    m_program->SetUniform("light.diffuse", m_light.diffuse);
    m_program->SetUniform("light.specular", m_light.specular);
    m_program->SetUniform("blinn", (m_blinn ? 1 : 0));

    m_program->SetUniform("material.diffuse", 0); // texture slot
    m_program->SetUniform("material.specular", 1);

    //

    m_lightingShadowProgram->SetUniform("material.diffuse", 0); // texture slot
    m_lightingShadowProgram->SetUniform("material.specular", 1);
}

void Context::UpdateCamera()
{
    m_camera.Front = rotate(mat4(1.0f), radians(m_camera.Yaw), vec3(0.0f, 1.0f, 0.0f)) *
                     rotate(mat4(1.0f), radians(m_camera.Pitch), vec3(1.0f, 0.0f, 0.0f)) *
                     vec4(0.0f, 0.0f, -1.0f, 0.0f); // 4차원 성분이 0이면 벡터, 1이면 점

    // 종횡비 4:3, 세로화각 45도의 원근 투영
    m_camera.projection = perspective(radians(45.0f), (float)m_width / (float)m_height, 0.01f, 100.0f);
    m_camera.view = lookAt(m_camera.Pos, m_camera.Pos + m_camera.Front, m_camera.Up);
}

void Context::DrawShadowedObjects(const mat4 &view, const mat4 &projection, const MaterialPtr &optionMat)
{
    objGround->Render(view, projection, optionMat);
    objBox1->Render(view, projection, optionMat);
    stencilBox->Render(view, projection, optionMat, m_simpleProgram, vec4(1.0f, 1.0f, 0.5f, 1.0f), 1.05f);
}

void Context::DrawShadowedObjects(const Camera &cam, const MaterialPtr &optionMat)
{
    DrawShadowedObjects(cam.view, cam.projection, optionMat);
}

void Context::GenerateShadowMap()
{
    auto lightView = lookAt(m_light.position,
                            m_light.position + m_light.direction,
                            vec3(0.0f, 1.0f, 0.0f));

    auto lightProjection = m_light.directional ? ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 30.0f)
                                               : perspective(radians((m_light.cutoff[0] + m_light.cutoff[1]) * 2.0f), 1.0f, 1.0f, 20.0f);

    m_shadowMap->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0,
               m_shadowMap->GetShadowMap()->GetWidth(),
               m_shadowMap->GetShadowMap()->GetHeight());

    // 광원에서 shadow depth map을 그림
    DrawShadowedObjects(lightView, lightProjection, shadowmapMaterial);
    //

    Framebuffer::BindToDefault();
    glViewport(0, 0, m_width, m_height);

    m_lightingShadowProgram->Use();
    m_lightingShadowProgram->SetUniform("viewPos", m_camera.Pos);
    m_lightingShadowProgram->SetUniform("light.directional", m_light.directional ? 1 : 0);
    m_lightingShadowProgram->SetUniform("light.position", m_light.position);
    m_lightingShadowProgram->SetUniform("light.direction", m_light.direction);
    m_lightingShadowProgram->SetUniform("light.cutoff", vec2(
                                                            cosf(radians(m_light.cutoff[0])),
                                                            cosf(radians(m_light.cutoff[0] + m_light.cutoff[1]))));
    m_lightingShadowProgram->SetUniform("light.attenuation", GetAttenuationCoeff(m_light.distance));
    m_lightingShadowProgram->SetUniform("light.ambient", m_light.ambient);
    m_lightingShadowProgram->SetUniform("light.diffuse", m_light.diffuse);
    m_lightingShadowProgram->SetUniform("light.specular", m_light.specular);
    m_lightingShadowProgram->SetUniform("blinn", (m_blinn ? 1 : 0));
    m_lightingShadowProgram->SetUniform("lightTransform", lightProjection * lightView);

    const int shadowMapTexNum = 9;
    glActiveTexture(GL_TEXTURE0 + shadowMapTexNum);
    m_shadowMap->GetShadowMap()->Bind();
    m_lightingShadowProgram->SetUniform("shadowMap", shadowMapTexNum);
    glActiveTexture(GL_TEXTURE0);

    // shadowed Material

    DrawShadowedObjects(m_camera);
}

void Context::RenderDeffered()
{
    glDisable(GL_BLEND); // 디퍼드 쉐이딩 때는 블렌딩 사용 불가

    m_deferGeoFramebuffer->Bind();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_width, m_height);

    objDeferredGround->Render(m_camera.view, m_camera.projection);
    objDeferredBox->Render(m_camera.view, m_camera.projection);
    m_model->Render(m_camera.view, m_camera.projection);

    m_ssaoFramebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_width, m_height);
    objSSAOPlane->Render(m_camera, m_deferGeoFramebuffer, m_ssaoNoiseTexture, vec2(m_width, m_height), m_ssaoRadius, m_ssaoSamples);

    m_ssaoBlurFramebuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_width, m_height);
    objBlurPlane->Render(m_ssaoFramebuffer->GetColorAttachment(0));

    //
    Framebuffer::BindToDefault();
    glViewport(0, 0, m_width, m_height);
    glClearColor(m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    objDeferredPlane->Render(m_camera, m_deferGeoFramebuffer, m_ssaoBlurFramebuffer, m_deferLights, m_useSsao);

    //// forward 쉐이딩 전환
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_deferGeoFramebuffer->Get());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    // read buffer의 뎁스 정보(GL_DEPTH_BUFFER_BIT)를 draw 버퍼에 복사함
    glBlitFramebuffer(0, 0, m_width, m_height,
                      0, 0, m_width, m_height,
                      GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Context::Render()
{
    RenderIMGUI();

    // m_framebuffer->Bind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    UpdateCamera();

    RenderDeffered();
    UpdateLight(m_camera.projection, m_camera.view);

    GenerateShadowMap();
    objSkybox->Render(m_camera.view, m_camera.projection);

    objPlane1->Render(m_camera);
    objPlane2->Render(m_camera);
    objPlane3->Render(m_camera);

    objCubemap->Render(m_camera);
    objGrass->Render(m_camera);
    objWall->Render(m_camera, m_light.position, m_wallMaterial);

    //// post process
    // Framebuffer::BindToDefault();
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    //// m_postProgram->Use();
    //// m_postProgram->SetUniform("transform",
    ////                           scale(mat4(1.0f), vec3(2.0f, 2.0f, 1.0f)));
    //// m_framebuffer->GetColorAttachment()->Bind();
    //// m_postProgram->SetUniform("tex", 0);
    //// m_postProgram->SetUniform("gamma", m_gamma);
    //// m_plane->Draw(m_postProgram.get());
}

void Context::RenderIMGUI()
{
    if (ImGui::Begin("UI window")) // 펼쳤을 때
    {
        if (ImGui::CollapsingHeader("light", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("l.directional", &m_light.directional);
            ImGui::DragFloat3("l.position", value_ptr(m_light.position), 0.01f);
            ImGui::DragFloat3("l.direction", value_ptr(m_light.direction), 0.01f);
            ImGui::DragFloat("l.distance", &m_light.distance, 0.5f, 0, 3000.0f);
            ImGui::DragFloat2("l.cutoff", value_ptr(m_light.cutoff), 0.5f, 0, 180.0f);
            ImGui::ColorEdit3("l.ambient", value_ptr(m_light.ambient));
            ImGui::ColorEdit3("l.diffuse", value_ptr(m_light.diffuse));
            ImGui::ColorEdit3("l.specular", value_ptr(m_light.specular));
            ImGui::Checkbox("flash light", &m_freshLightMode);
            ImGui::Checkbox("l.blinn", &m_blinn);
            ImGui::Checkbox("use SSao", &m_useSsao);
            ImGui::DragFloat("ssao radius", &m_ssaoRadius, 0.01f, 0.f, 5.0f);
        }

        ImGui::Checkbox("animation", &m_animation);

        if (ImGui::ColorEdit4("clear color", value_ptr(m_clearColor)))
        {
            glClearColor(m_clearColor.x, m_clearColor.y, m_clearColor.z, m_clearColor.w);
        }
        ImGui::DragFloat("gamma", &m_gamma, 0.01f, 0.0f, 2.0f);

        ImGui::Separator();
        ImGui::DragFloat3("camera pos", value_ptr(m_camera.Pos), 0.01f);
        ImGui::DragFloat("camera yaw", &m_camera.Yaw), 0.5f;
        ImGui::DragFloat("camera pitch", &m_camera.Pitch, 0.5f, -89, 89);
        ImGui::Separator();
        if (ImGui::Button("reset camera"))
        {
            m_camera.Yaw = 0;
            m_camera.Pitch = 0;
            m_camera.Pos = vec3(0, 0, 3);
        }
        ImGui::Text("This is first text...");
    }

    float aspectRatio = (float)m_width / m_height;
    // ImGui::Image((ImTextureID)m_framebuffer->GetColorAttachment()->Get(),
    //              ImVec2(150 * aspectRatio, 150));
    ImGui::Image((ImTextureID)m_shadowMap->GetShadowMap()->Get(),
                 ImVec2(256, 256), ImVec2(0, 1), ImVec2(1, 0));
    ImGui::End();

    if (ImGui::Begin("G-Buffers"))
    {
        const char *bufferNames[] = {
            "position",
            "normal",
            "albedo/specular",
        };
        static int bufferSelect = 0;
        ImGui::Combo("buffer", &bufferSelect, bufferNames, 3);
        float width = ImGui::GetContentRegionAvailWidth(); // 화면에 그릴 수 있는 window size
        float height = width * ((float)m_height / (float)m_width);
        auto selectedAttachment =
            m_deferGeoFramebuffer->GetColorAttachment(bufferSelect);
        ImGui::Image((ImTextureID)selectedAttachment->Get(),
                     ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();

    if (ImGui::Begin("SSAO"))
    {
        const char *bufferNames[] = {"original", "blurred"};
        static int bufferSelect = 0;
        ImGui::Combo("buffer", &bufferSelect, bufferNames, 2);

        float width = ImGui::GetContentRegionAvailWidth();
        float height = width * ((float)m_height / (float)m_width);
        auto selectedAttachment = bufferSelect == 0 ? m_ssaoFramebuffer->GetColorAttachment() : m_ssaoBlurFramebuffer->GetColorAttachment();

        ImGui::Image((ImTextureID)selectedAttachment->Get(),
                     ImVec2(width, height), ImVec2(0, 1), ImVec2(1, 0));
    }
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

    auto cameraRight = normalize(cross(m_camera.Up, -m_camera.Front));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_camera.Pos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_camera.Pos -= cameraSpeed * cameraRight;

    auto cameraUp = normalize(cross(-m_camera.Front, cameraRight));
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

    m_framebuffer = Framebuffer::Create({
        Texture::Create(width, height, GL_RGBA),
    });

    m_deferGeoFramebuffer = Framebuffer::Create({
        Texture::Create(width, height, GL_RGBA16F, GL_FLOAT),      // position
        Texture::Create(width, height, GL_RGBA16F, GL_FLOAT),      // normal
        Texture::Create(width, height, GL_RGBA, GL_UNSIGNED_BYTE), // albedo
    });

    m_ssaoFramebuffer = Framebuffer::Create({
        Texture::Create(width, height, GL_RED),
    });

    m_ssaoBlurFramebuffer = Framebuffer::Create({
        Texture::Create(width, height, GL_RED),
    });
}
void Context::MouseMove(double x, double y)
{
    if (!m_camera.Control)
        return;

    static vec2 prevPos = vec2((float)x, (float)y);
    auto pos = vec2((float)x, (float)y);
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
            m_prevMousePos = vec2((float)x, (float)y);
            m_camera.Control = true;
        }
        else if (action == GLFW_RELEASE)
        {
            m_camera.Control = false;
        }
    }
}
