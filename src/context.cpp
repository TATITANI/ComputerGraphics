#include "context.h"
#include "image.h"

ContextUPtr Context::Create()
{
    auto context = ContextUPtr(new Context());
    if (!context->Init())
        return nullptr;
    return std::move(context);
}

bool Context::Init()
{
    //[x, y, z, s, t]
    float vertices[] = {
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, //
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  //
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   //
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  //

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, //
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  //
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   //
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  //

        -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   //
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  //
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  //

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   //
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  //
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  //

        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, //
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  //
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   //
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  //

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, //
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  //
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   //
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,  //
    };

    uint32_t indices[] = {
        0, 2, 1, 2, 0, 3,       //
        4, 5, 6, 6, 7, 4,       //
        8, 9, 10, 10, 11, 8,    //
        12, 14, 13, 14, 12, 15, //
        16, 17, 18, 18, 19, 16, //
        20, 22, 21, 22, 20, 23, //
    };

    // VBO
    m_vertexBuffer = Buffer::CreateWithData(GL_ARRAY_BUFFER, GL_STATIC_DRAW, vertices, sizeof(vertices));

    // VAO
    const int stride = 5;
    m_vertexLayout = VertexLayout::Create();
    m_vertexLayout->SetAttrib(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, 0); // pos
    // m_vertexLayout->SetAttrib(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * stride, sizeof(float) * 3); // color
    m_vertexLayout->SetAttrib(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * stride, sizeof(float) * 3); // tex

    // 인덱스 버퍼
    m_indexBuffer = Buffer::CreateWithData(GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices, sizeof(indices));

    // GL_STREAM_DRAW: the data is set only once and used by the GPU at most a few times.
    // GL_STATIC_DRAW: the data is set only once and used many times.
    // GL_DYNAMIC_DRAW: the data is changed a lot and used many times.

    ShaderPtr vertShader = Shader::CreateFromFile("./shader/texture.vs", GL_VERTEX_SHADER);
    ShaderPtr fragShader = Shader::CreateFromFile("./shader/texture.fs", GL_FRAGMENT_SHADER);
    if (!vertShader || !fragShader)
        return false;

    SPDLOG_INFO("vertex shader id: {}", vertShader->Get());
    SPDLOG_INFO("fragment shader id: {}", fragShader->Get());

    m_program = Program::Create({fragShader, vertShader});
    if (!m_program)
        return false;
    SPDLOG_INFO("program id: {}", m_program->Get());

    // uniform 변수 설정
    // auto loc = glGetUniformLocation(m_program->Get(), "color");
    // m_program->Use();
    // glUniform4f(loc, 1.0f, 1.0f, 0.0f, 1.0f);

    glClearColor(0.1f, 0.2f, 0.3f, 0.0f);

    ////// texture
    auto image = ImagePtr(Image::Load("./image/container.jpg"));
    // auto image = ImagePtr(Image::Create(512, 512));
    // image->SetCheckImage(16, 16);
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

    m_program->SetUniform("tex", 0);
    m_program->SetUniform("tex2", 1);

    // x축으로 -55도 회전
    auto model = glm::rotate(glm::mat4(1.0f),
                             glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    // 카메라는 원점으로부터 z축 방향으로 -3만큼 떨어짐
    auto view = glm::translate(glm::mat4(1.0f),
                               glm::vec3(0.0f, 0.0f, -3.0f));
    // 종횡비 4:3, 세로화각 45도의 원근 투영
    auto projection = glm::perspective(glm::radians(45.0f),
                                       (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 10.0f);
    auto transform = projection * view * model;
    m_program->SetUniform("transform", transform);

    return true;
}

void Context::Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    m_program->Use();

    auto projection = glm::perspective(glm::radians(45.0f),
                                       (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 20.0f);
    auto view = glm::translate(glm::mat4(1.0f),
                               glm::vec3(0.0f, 0.0f, -3.0f));

    // auto model = glm::rotate(glm::mat4(1.0f),
    //                          glm::radians((float)glfwGetTime() * 120.0f),
    //                          glm::vec3(1.0f, 0.5f, 0.0f));
    // auto transform = projection * view * model;
    // m_program->SetUniform("transform", transform);

    for (size_t i = 0; i < cubePositions.size(); i++)
    {
        auto &pos = cubePositions[i];
        auto model = glm::translate(glm::mat4(1.0f), pos);
        model = glm::rotate(model,
                            glm::radians((float)glfwGetTime() * 120.0f + 20.0f * (float)i),
                            glm::vec3(1.0f, 0.5f, 0.0f));
        auto transform = projection * view * model;
        m_program->SetUniform("transform", transform);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }

    // primitive 타입, ebo내 index 개수, index 데이터형, ebo 첫 데이터의 오프셋
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}
