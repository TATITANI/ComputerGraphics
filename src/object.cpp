#include "context.h"
#include "object.h"

mat4 Transform::GetTransform()
{
    return translate(mat4(1.0f), pos) *
           rotate(mat4(1.0f), radians(rot.x), vec3(1.0f, 0.0f, 0.0f)) *
           rotate(mat4(1.0f), radians(rot.y), vec3(0.0f, 1.0f, 0.0f)) *
           rotate(mat4(1.0f), radians(rot.z), vec3(0.0f, 0.0f, 1.0f)) *
           scale(mat4(1.0f), scaleVec);
}

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
    instanceVAO->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                           offsetof(Vertex, normal));
    instanceVAO->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                           offsetof(Vertex, texCoord));

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
    auto modelTransform = trf.GetTransform(); // world
    auto transform = projection * view * modelTransform; // clip space
    currentProgram->SetUniform("transform", transform);
    currentProgram->SetUniform("modelTransform", modelTransform);
}

void Object::Draw()
{
    if (isInstance)
        mesh->Draw(currentProgram.get(), instanceVAO.get(), posBuffer->GetCount());
    else
        mesh->Draw(currentProgram.get());
}

void Object::UseProgram(const ProgramPtr &_program, const MaterialPtr &mat)
{
    currentProgram = _program;
    _program->Use();
    if (mat)
        mat->SetToProgram(_program.get());
}

void Object::UseProgram(const MaterialPtr &mat)
{
    UseProgram(this->defaultProgram, mat);
}

void Object::Render(const Camera &cam, const MaterialPtr &mat, const ProgramPtr &optionPgm)
{
    Render(cam.view, cam.projection, mat, optionPgm);
}

void Object::Render(const mat4 &view, const mat4 &projection,
                    const MaterialPtr &mat, const ProgramPtr &optionPgm)
{
    UseProgram(optionPgm ? optionPgm : defaultProgram, mat);
    Update(view, projection);
    Draw();
}

void Cubemap::Render(const Camera &cam, const MaterialPtr &mat, const ProgramPtr &optionPgm)
{
    UseProgram(optionPgm ? optionPgm : defaultProgram, mat);
    Update(cam);
    Draw();
}

void Cubemap::Update(const Camera &cam)
{
    currentProgram->SetUniform("model", trf.GetTransform());
    currentProgram->SetUniform("view", cam.view);
    currentProgram->SetUniform("projection", cam.projection);
    currentProgram->SetUniform("cameraPos", cam.Pos);
    currentProgram->SetUniform("skybox", 0);
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

    outlineProgram->Use();
    outlineProgram->SetUniform("color", color);
    outlineProgram->SetUniform("transform", trf * scale(mat4(1.0f), vec3(outlineSize)));
    mesh->Draw(outlineProgram.get());

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
}

void StencilBox::UseProgram(const ProgramPtr &optionPgm, const ProgramPtr &_outlinePgm)
{
    obj.UseProgram(optionPgm ? optionPgm : obj.defaultProgram);
    outlineProgram = _outlinePgm;
}

void StencilBox::Render(const Camera &cam, const ProgramPtr &optionPgm, ProgramPtr &_outlinePgm,
                        vec4 &color, float outlineSize)
{
    Render(cam.view, cam.projection, optionPgm, _outlinePgm,
           color, outlineSize);
}

void StencilBox::Render(const mat4 &view, const mat4 &projection, const ProgramPtr &optionPgm,
                        ProgramPtr &_outlinePgm, vec4 &color, float outlineSize)
{
    UseProgram(optionPgm, _outlinePgm);
    Update(view, projection);
    Draw(color, outlineSize);
}
