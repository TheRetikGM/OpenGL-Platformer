#include "PostProcessor.h"
#include "game/game.h"
#include "tileCamera2D.h"
#include "glad/glad.h"

PostProcessor::PostProcessor(Shader& s, unsigned int width, unsigned int height) : shader(s)
{
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    color_buf.Internal_format = GL_RGBA;
    color_buf.Image_format = GL_RGBA;
    color_buf.Generate(width, height, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_buf.ID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("PostProcessor::PostProcessor(): Failed to initialize FBO.");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    init_render_data();

    s.Use().SetInt("scene", 0);
}
PostProcessor::~PostProcessor()
{
    glDeleteFramebuffers(1, &FBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &color_buf.ID);
}
void PostProcessor::BeginRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glClearColor(vClearColor.x, vClearColor.y, vClearColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void PostProcessor::EndRender()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void PostProcessor::Render(float time)
{
    shader.Use();
    shader.SetFloat("radius", float(M_SQRT2));
    glActiveTexture(GL_TEXTURE0);
    color_buf.Bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
void PostProcessor::init_render_data()
{
    unsigned int VBO;
    float vertices[] = {
        // pos          // tex
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void PostProcessor::HandleResize(unsigned int width, unsigned int height)
{
    color_buf.Generate(width, height, NULL);
}
