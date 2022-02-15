#pragma once
#include "shader.h"
#include "texture.h"
#include <functional>

enum class AnimStepMethod
{
    linear = 0,
    quadratic = 1,
    cubatic = 2     // FIXME : what is this name ...
};

class PostProcessor
{
public:
    PostProcessor(Shader& shader, unsigned int width, unsigned int height);
    ~PostProcessor();

    void BeginRender();
    void EndRender();
    void Render();
    void Update(float dt);

    void Restart(std::function<void()> onEnd);

    void HandleResize(unsigned int width, unsigned int height);
    inline void SetClearColor(glm::vec3 color) { vClearColor = color; }

    void SetAnimStepMethod(AnimStepMethod method);

protected:
    unsigned int FBO;
    unsigned int VAO;

    Shader shader;
    Texture2D color_buf;
    glm::vec3 vClearColor = glm::vec3(0.0f);

    bool bFadeOut = false;
    float fCircleRadius = 0.0f;
    float fCurrentCircleRadius = 0.0f;
    float fTimeCurrent = 0.0f;
    float fAnimDuration = 1.0f;
    bool bOngoingAnimation = false;
    std::function<void()> onEnd = [](){};

    float (PostProcessor::*animStepFunc)(float) = &PostProcessor::cubatic_anim;
    float linear_anim(float time);
    float quadratic_anim(float time);
    float cubatic_anim(float time);

    void init_render_data();
    float get_next_r(float time);
};