#include "sprite_renderer.h"
#include "config.h"
#include "tileCamera2D.h"

SpriteRenderer::SpriteRenderer(Shader shader)
{
	this->shader = shader;
	initRenderData();
}
SpriteRenderer::~SpriteRenderer()
{
	glDeleteVertexArrays(1, &this->quadVAO);
}

SpriteRenderer& SpriteRenderer::DrawSprite(Texture2D texture, glm::vec2 position, glm::vec2 size, float rotate, glm::vec3 color)
{
	this->shader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0));
	model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
	model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
	model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

	this->shader.SetMat4("model", TileCamera2D::GetViewMatrix() * model);
	this->shader.SetVec3f("spriteColor", color);
	this->shader.SetVec4f("spriteScaleOffset", glm::vec4(1.0f, 1.0f, 0.0f, 0.0f));

	glActiveTexture(GL_TEXTURE0);
	texture.Bind();

	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	return *this;
}
SpriteRenderer& SpriteRenderer::DrawPartialSprite(Texture2D texture, glm::vec2 vPartOffset, glm::vec2 vPartSize, glm::vec2 vPosition, glm::vec2 vSize, float fRotate, glm::vec3 vColor)
{
	this->shader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(vPosition, 0.0f));
	model = glm::translate(model, glm::vec3(0.5f * vSize.x, 0.5f * vSize.y, 0.0f));
	model = glm::rotate(model, glm::radians(fRotate), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * vSize.x, -0.5f * vSize.y, 0.0f));
	model = glm::scale(model, glm::vec3(vSize.x, vSize.y, 1.0f));

	this->shader.SetMat4("model", TileCamera2D::GetViewMatrix() * model);
	this->shader.SetVec3f("spriteColor", vColor);

	glm::vec2 vSpriteScale = vPartSize / glm::vec2(texture.Width, texture.Height);
	glm::vec2 vSpriteOffset = vPartOffset / glm::vec2(texture.Width, texture.Height);
	this->shader.SetVec4f("spriteScaleOffset", glm::vec4(vSpriteScale, vSpriteOffset));

	glActiveTexture(GL_TEXTURE0);
	texture.Bind();

	glBindVertexArray(this->quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);

	return *this;
}
void SpriteRenderer::initRenderData()
{
	unsigned int VBO;
	float vertices[] = {
		// pos    // tex
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f,			
	};	

	glGenVertexArrays(1, &this->quadVAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(this->quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}
