#pragma once
#include <glm/glm.hpp>
#include "basic_renderer.h"

class ICamera
{
public:
    glm::vec2   Position;
    float       MoveSpeed;        

    virtual glm::mat4 GetViewMatrix() { return glm::mat4(1.0f); };
    virtual void ProccessKeyboard(glm::vec2 direction, float delta) { Position += MoveSpeed * delta * direction; };
    virtual void Rotate(float angle) {};
    virtual glm::vec2 GetRight() { return glm::vec2(0.0f, 0.0f);  };
    virtual void SetRight(glm::vec2 right) {};
    virtual void RenderAtPosition(BasicRenderer* renderer, br_Shape shape, glm::vec3 color = glm::vec3(1.0f)) {};
protected:    
    ICamera(glm::vec2 pos, float moveSpeed) : Position(pos), MoveSpeed(moveSpeed) {}
};