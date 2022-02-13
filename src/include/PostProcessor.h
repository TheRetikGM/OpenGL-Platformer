#pragma once
#include "shader.h"
#include "texture.h"

class PostProcessor
{
public:
    PostProcessor(Shader& shader, unsigned int width, unsigned int height);
    ~PostProcessor();

    void BeginRender();
    void EndRender();
    void Render(float time);

    void HandleResize(unsigned int width, unsigned int height);
    inline void SetClearColor(glm::vec3 color) { vClearColor = color; }

protected:
    unsigned int FBO;
    unsigned int VAO;

    Shader shader;
    Texture2D color_buf;
    glm::vec3 vClearColor = glm::vec3(0.0f);

    void init_render_data();
};