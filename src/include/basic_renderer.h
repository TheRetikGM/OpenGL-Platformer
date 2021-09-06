#pragma once
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader.h"

enum class br_Shape : int
{
	square, triangle, circle, rectangle
};
struct br_RenderInfo
{
	unsigned int VBO;
	unsigned int VAO;
	GLenum mode;
	unsigned int n_strips;
	br_RenderInfo() : VBO(0), VAO(0), mode(0), n_strips(0) {}
};

class BasicRenderer
{
public:
	BasicRenderer(Shader shader);
	~BasicRenderer();

	void RenderShape(br_Shape shape, glm::vec2 position, glm::vec2 scale = glm::vec2(1.0f), float rotate_radians = 0.0f, glm::vec3 color = glm::vec3(1.0f));
	void SetLineWidth(float width);
	float GetLineWidth() const { return lineWidth; }
	Shader GetShader() const { return shader;  }
private:
	std::unordered_map<br_Shape, br_RenderInfo> mp_shape_info;
	Shader shader;
	float lineWidth;

	void initShape(br_Shape shape);		
};