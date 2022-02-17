#include "PostProcessor.h"
#include "game/game.h"
#include "tileCamera2D.h"
#include "glad/glad.h"

PostProcessor::PostProcessor(Shader& s, unsigned int width, unsigned int height) : shader(s)
{
    // Generate framebuffer and texture used as color buffer.
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
    s.SetMat4("projection", Game::ProjectionMatrix);
    s.SetVec2f("center_pos", glm::vec2(width, height) * 0.5f);
    fCurrentCircleRadius = fCircleRadius = glm::length(glm::vec2(width, height) * 0.5f);
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
void PostProcessor::Restart(std::function<void()> onEnd)
{
    bOngoingAnimation = true;
    fTimeCurrent = 0.0f;
    fCurrentCircleRadius = fCircleRadius;
    this->onEnd = onEnd;
    bFadeOut = false;
}
void PostProcessor::Update(float dt)
{
    if (bOngoingAnimation)
    {
        fTimeCurrent += dt;
        float scale = (this->*animStepFunc)(fTimeCurrent);
        if (bFadeOut)
            scale = 1.0f - scale;
        if (scale < 0.0f || scale > 1.0f || fTimeCurrent > fAnimDuration)
        {
            if (!bFadeOut)
            {
                onEnd();
                fTimeCurrent = 0.0f;
                bFadeOut = true;
            }
            else
            {
                bOngoingAnimation = false;
                fCurrentCircleRadius = fCircleRadius;
            }
        }
        else
            fCurrentCircleRadius = fCircleRadius * scale;
    }
}
float PostProcessor::linear_anim(float time)
{
    return -(1.0f / fAnimDuration) * time + 1.0f;
}
float PostProcessor::quadratic_anim(float time)
{
    return -(1.0f / (fAnimDuration * fAnimDuration)) * time * time + 1.0f;
}
float PostProcessor::cubatic_anim(float time)
{
    return -std::pow((1.0f / fAnimDuration) * time - 1.0f, 5);
}
void PostProcessor::Render()
{
    shader.Use();
    shader.SetFloat("radius", fCurrentCircleRadius);

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(Game::ScreenSize, 0.0f));
    shader.SetMat4("model", model);

    glActiveTexture(GL_TEXTURE0);
    color_buf.Bind();
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
void PostProcessor::SetAnimStepMethod(AnimStepMethod method)
{
    switch (method)
    {
    case AnimStepMethod::linear:
        animStepFunc = &PostProcessor::linear_anim;
        break;
    case AnimStepMethod::quadratic:
        animStepFunc = &PostProcessor::quadratic_anim;
        break;
    case AnimStepMethod::cubatic:
        animStepFunc = &PostProcessor::cubatic_anim;
        break;
    default:
        break;
    }
}
void PostProcessor::init_render_data()
{
    unsigned int VBO;
    float vertices[] = {
        // pos    // tex
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f,			
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
    // Resize color buffer to match screen size.
    color_buf.Generate(width, height, NULL);
    // Renew render variables.
    shader.Use().SetMat4("projection", Game::ProjectionMatrix);
    shader.SetVec2f("center_pos", glm::vec2(width, height) * 0.5f);
    // Set radius of the circle to it's initial state.
    fCurrentCircleRadius = fCircleRadius = glm::length(glm::vec2(width, height) * 0.5f);
}
