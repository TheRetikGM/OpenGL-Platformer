#pragma once
#include "shader.h"
#include "texture.h"
#include <glm/glm.hpp>

class SpriteRenderer
{
public:
	SpriteRenderer(Shader shader);
	~SpriteRenderer();

	void DrawSprite(Texture2D texture, glm::vec2 position,
		glm::vec2 size = glm::vec2(10.0f, 10.0f), float rotate = 0.0f,
		glm::vec3 color = glm::vec3(1.0f));

	// Draws only part of the texture.
	void DrawPartialSprite(Texture2D texture
						 , glm::vec2 vPartOffset	// Part offset in pixels
						 , glm::vec2 vPartSize		// part size in pixels
						 , glm::vec2 vPosition		// Pozition in pixels
						 , glm::vec2 vSize			// Wanted size in pixels
						 , float fRotate = 0.0f
						 , glm::vec3 vColor = glm::vec3(1.0f)
	);

	Shader GetShader() const { return shader; };
private:
	Shader shader;
	unsigned int quadVAO;

	void initRenderData();
};