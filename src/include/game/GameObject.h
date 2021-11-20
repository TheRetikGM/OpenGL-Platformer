#pragma once
#include <glm/glm.hpp>
#include "basic_renderer.h"

class GameObject
{
public:
	glm::vec2 Position;	
	glm::vec2 Velocity;
	glm::vec2 Acceleration;
	glm::vec2 Size;
	glm::vec3 Color;

	virtual glm::vec2 GetScreenPosition() { return glm::vec2(0.0f, 0.0f); }

protected:	

	GameObject(glm::vec2 position, glm::vec2 velocity, glm::vec2 size, glm::vec3 color)
		: Position(position), Velocity(velocity), Size(size), Color(color), Acceleration(glm::vec2(0.0f))
	{}
};