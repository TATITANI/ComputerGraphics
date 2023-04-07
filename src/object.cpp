#include "context.h"
#include "object.h"

void Object::ActiveInstancing(size_t size, int atbIndex, int atbCount, int atbDivisor)
{
    isInstance = true;

    positions.resize(size);
    for (size_t i = 0; i < positions.size(); i++)
    {
        positions[i].x = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 5.0f;
        positions[i].z = ((float)rand() / (float)RAND_MAX * 2.0f - 1.0f) * 5.0f;
        positions[i].y = glm::radians((float)rand() / (float)RAND_MAX * 360.0f);
    }
    instanceVAO = VertexLayout::Create(); // VAO
    instanceVAO->Bind();

    mesh->BindVertexBuffer();
    instanceVAO->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    instanceVAO->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, normal));
    instanceVAO->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), offsetof(Vertex, texCoord));
    instanceVAO->SetAttrib(3, 3, GL_FLOAT, false, sizeof(Vertex), offsetof(Vertex, tangent));

    posBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW,
                                       positions.data(), sizeof(glm::vec3), positions.size());
    posBuffer->Bind();
    instanceVAO->SetAttrib(atbIndex, atbCount, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
    // index번 Attribute는 인스턴스가 divisor번 바뀔때마다 변경
    glVertexAttribDivisor(atbIndex, atbDivisor);
    mesh->BindIndexBuffer();
}

void Object::Update(const Camera &cam)
{
    Update(cam.view, cam.projection);
}

void Object::Update(const mat4 &view, const mat4 &projection)
{
    auto modelTransform = trf.GetTransform();            // world
    auto transform = projection * view * modelTransform; // clip space

    currentMaterial->SetProperty("transform", transform);
    currentMaterial->SetProperty("modelTransform", modelTransform);
}

void Object::Draw()
{
    if (currentMaterial)
        currentMaterial->Apply();

    if (isInstance)
        mesh->Draw(instanceVAO.get(), posBuffer->GetCount());
    else
        mesh->Draw();
}

void Object::Render(const Camera &cam, const MaterialPtr &optionMat)
{
    SetCurrentMaterial(optionMat ? optionMat : material);
    Update(cam);
    Draw();
}

void Object::Render(const mat4 &view, const mat4 &projection,
                    const MaterialPtr &optionMat)
{
    SetCurrentMaterial(optionMat ? optionMat : material);
    Update(view, projection);
    Draw();
}

void Cubemap::Update(const Camera &cam)
{
    currentMaterial->SetProperty("model", trf.GetTransform());
    currentMaterial->SetProperty("view", cam.view);
    currentMaterial->SetProperty("projection", cam.projection);
    currentMaterial->SetProperty("cameraPos", cam.Pos);
    currentMaterial->SetProperty("skybox", 0);
}

void Wall::Update(const Camera &cam, const vec3 &lightPos)
{
    currentMaterial->SetProperty("viewPos", cam.Pos);
    currentMaterial->SetProperty("lightPos", lightPos);

    auto modelTransform = trf.GetTransform();                    // world
    auto transform = cam.projection * cam.view * modelTransform; // clip space
    currentMaterial->SetProperty("transform", transform);
    currentMaterial->SetProperty("modelTransform", modelTransform);
}

void Wall::Render(const Camera &cam, const vec3 &lightPos, const MaterialPtr &optionMat)
{
    SetCurrentMaterial(optionMat ? optionMat : material);
    Update(cam, lightPos);
    Draw();
}

void DeferredPlane::Render(const Camera &cam, const FramebufferPtr &gepBuf,const FramebufferPtr &blurBuf, const vector<DeferLight> &_lights, bool useSsao)
{
    currentMaterial->SetProperty("gPosition", gepBuf->GetColorAttachment(0));
    currentMaterial->SetProperty("gNormal", gepBuf->GetColorAttachment(1));
    currentMaterial->SetProperty("gAlbedoSpec", gepBuf->GetColorAttachment(2));

    for (size_t i = 0; i < _lights.size(); i++)
    {
        auto posName = fmt::format("lights[{}].position", i);
        auto colorName = fmt::format("lights[{}].color", i);
        currentMaterial->SetProperty(posName, _lights[i].position);
        currentMaterial->SetProperty(colorName, _lights[i].color);
    }

    currentMaterial->SetProperty("ssao", blurBuf->GetColorAttachment());
    currentMaterial->SetProperty("useSsao", useSsao ? 1 : 0);

    currentMaterial->SetProperty("transform", trf.GetTransform());

    Draw();
}

void SSAOPlane::Render(const Camera &cam, const FramebufferPtr &buf, const TexturePtr &noiseTex, const vec2 &windowSize,
                       const float &radius, const vector<vec3> &samples)
{
    currentMaterial->SetProperty("gPosition", buf->GetColorAttachment(0));
    currentMaterial->SetProperty("gNormal", buf->GetColorAttachment(1));
    currentMaterial->SetProperty("texNoise", noiseTex);
    currentMaterial->SetProperty("noiseScale", vec2((float)windowSize.x / (float)noiseTex->GetWidth(),
                                                    (float)windowSize.y / (float)noiseTex->GetHeight()));

    currentMaterial->SetProperty("view", cam.view);
    currentMaterial->SetProperty("transform", trf.GetTransform());

    currentMaterial->SetProperty("radius", radius);
    for (size_t i = 0; i < samples.size(); i++)
    {
        auto sampleName = fmt::format("samples[{}]", i);
        currentMaterial->SetProperty(sampleName, samples[i]);
    }

    Draw();
}

void BlurPlane::Render(const TexturePtr &tex)
{
    currentMaterial->SetProperty("transform", trf.GetTransform());
    currentMaterial->SetProperty("tex", tex);

    Draw();
}

void StencilBox::Update(const mat4 &view, const mat4 &projection)
{
    obj.Update(view, projection);
    trf = projection * view * obj.trf.GetTransform();
}

void StencilBox::Draw(vec4 &color, float outlineSize)
{
    // 스텐실 테스트
    glEnable(GL_STENCIL_TEST);
    // @param : 스텐실 테스트 실패, 스텐실 테스트는 통과했지만 depth test실패, 성공시 이벤트
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF); // 업데이트 되는 스텐실 버퍼의 비트 설정. 0xFF : 모든 비트 기록.
    obj.Draw();

    glStencilFunc(GL_NOTEQUAL, 1, 0xFF); // 1이 아닌 프래그먼트만 그림
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    outlinePgm->Use();
    outlinePgm->SetUniform("color", color);
    outlinePgm->SetUniform("transform", trf * scale(mat4(1.0f), vec3(outlineSize)));
    mesh->Draw();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}

void StencilBox::Render(const Camera &cam, const MaterialPtr &optionMat, ProgramPtr &_outlinePgm,
                        vec4 &color, float outlineSize)
{
    Render(cam.view, cam.projection, optionMat, _outlinePgm,
           color, outlineSize);
}

void StencilBox::Render(const mat4 &view, const mat4 &projection, const MaterialPtr &optionMat,
                        ProgramPtr &_outlinePgm, vec4 &color, float outlineSize)
{
    obj.SetCurrentMaterial(optionMat ? optionMat : obj.material);
    outlinePgm = _outlinePgm;
    Update(view, projection);
    Draw(color, outlineSize);
}
