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

void Object::Update(Camera &cam)
{
    auto modelTransform = trf.GetTransform();
    auto transform = cam.projection * cam.view * modelTransform;
    program->SetUniform("transform", transform);
    program->SetUniform("modelTransform", modelTransform);
}

void Object::Draw()
{
    if (isInstance)
        mesh->Draw(program.get(), instanceVAO.get(), posBuffer->GetCount());
    else
        mesh->Draw(program.get());
}

void Object::AttatchProgram(ProgramPtr &_program, MaterialPtr mat)
{
    program = _program;
    program->Use();

    if (mat)
        mat->SetToProgram(program.get());
}

void Object::Render(Camera &cam, ProgramPtr &_program, MaterialPtr mat)
{
    AttatchProgram(_program, mat);
    Update(cam);
    Draw();
}

void Cubemap::Update(Camera &cam)
{
    program->SetUniform("model", trf.GetTransform());
    program->SetUniform("view", cam.view);
    program->SetUniform("projection", cam.projection);
    program->SetUniform("cameraPos", cam.Pos);
    program->SetUniform("skybox", 0);
}

void StencilBox::Update(Camera &cam)
{
    obj.Update(cam);
    trf = cam.projection * cam.view * obj.trf.GetTransform();
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

void StencilBox::AttatchProgram(ProgramPtr &_objPgm, ProgramPtr &_outlinePgm)
{
    obj.AttatchProgram(_objPgm);
    outlineProgram = _outlinePgm;
}

void StencilBox::Render(Camera &cam, ProgramPtr &_objPgm, ProgramPtr &_outlinePgm,
                        vec4 &color, float outlineSize)
{
    AttatchProgram(_objPgm, _outlinePgm);
    Update(cam);
    Draw(color, outlineSize);
}
