#include "basic_renderer.h"
#include <vector>
#include "camera/tileCamera2D.h"

BasicRenderer::BasicRenderer(Shader shader)
	: mp_shape_info(), shader(shader), lineWidth(1.0f)
{
	mp_shape_info[br_Shape::triangle] = br_RenderInfo();
	mp_shape_info[br_Shape::square] = br_RenderInfo();
	mp_shape_info[br_Shape::circle] = br_RenderInfo();
	mp_shape_info[br_Shape::rectangle] = br_RenderInfo();
}
BasicRenderer::~BasicRenderer()
{
	for (auto& i : mp_shape_info)
	{
		if (i.second.VBO != 0)
		{
			glDeleteBuffers(1, &i.second.VBO);
			glDeleteVertexArrays(1, &i.second.VAO);
		}
	}
	if (line_VBO != 0)
	{
		glDeleteBuffers(1, &line_VBO);
		glDeleteVertexArrays(1, &line_VAO);
	}
}
void BasicRenderer::SetLineWidth(float width)
{
	glLineWidth(width);
	lineWidth = width;
}

void BasicRenderer::RenderShape(br_Shape shape, glm::vec2 position, glm::vec2 scale, float rotate_radians, glm::vec3 color)
{
	if (mp_shape_info[shape].VBO == 0)
		initShape(shape);

	this->shader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0));
	model = glm::translate(model, glm::vec3(0.5f * scale.x, 0.5f * scale.y, 0.0f));
	model = glm::rotate(model, rotate_radians, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::translate(model, glm::vec3(-0.5f * scale.x, -0.5f * scale.y, 0.0f));
	model = glm::scale(model, glm::vec3(scale, 1.0f));

	this->shader.SetMat4("model", TileCamera2D::GetViewMatrix() * model);
	this->shader.SetVec3f("color", color);

	glBindVertexArray(mp_shape_info[shape].VAO);
	glDrawArrays(mp_shape_info[shape].mode, 0, mp_shape_info[shape].n_strips);
	glBindVertexArray(0);
}
void BasicRenderer::RenderShape(br_Shape shape, glm::mat4 customModel, glm::vec3 color)
{
	if (mp_shape_info[shape].VBO == 0)
		initShape(shape);

	this->shader.Use();
	this->shader.SetMat4("model", TileCamera2D::GetViewMatrix() * customModel);
	this->shader.SetVec3f("color", color);

	glBindVertexArray(mp_shape_info[shape].VAO);
	glDrawArrays(mp_shape_info[shape].mode, 0, mp_shape_info[shape].n_strips);
	glBindVertexArray(0);
}
void BasicRenderer::RenderLine(glm::vec2 p1, glm::vec2 p2, glm::vec3 color)
{
	if (line_VBO == 0)
		initLineBuffers();

	glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
	float arr[4] = { p1.x, p1.y, p2.x, p2.y };
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(arr), arr);

	this->shader.Use();
	this->shader.SetMat4("model", glm::mat4(1.0f));
	this->shader.SetVec3f("color", color);

	glBindVertexArray(line_VAO);
	glDrawArrays(GL_LINES, 0, 2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void BasicRenderer::initLineBuffers()
{
	glGenBuffers(1, &line_VBO);
	glGenVertexArrays(1, &line_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, line_VBO);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

	glBindVertexArray(line_VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void initBuffers(br_RenderInfo& inf, size_t sizeof_vertices, float* vertices, GLenum mode, unsigned int n_strips)
{
	glGenBuffers(1, &inf.VBO);
	glGenVertexArrays(1, &inf.VAO);

	glBindBuffer(GL_ARRAY_BUFFER, inf.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof_vertices, vertices, GL_STATIC_DRAW);

	glBindVertexArray(inf.VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	inf.mode = mode;
	inf.n_strips = n_strips;
}
void BasicRenderer::initShape(br_Shape shape)
{	
	if (shape == br_Shape::triangle)
	{		
		float vertices[] = {
			0.5f, 0.0f,
			0.0f, 1.0f,
			1.0f, 1.0f
		};
		br_RenderInfo& inf = mp_shape_info[shape];
		initBuffers(inf, sizeof(vertices), vertices, GL_TRIANGLES, 3);
	}
	else if (shape == br_Shape::square)
	{
		float vertices[] = {
			0.0f, 0.0f,
			0.0f, 1.0f,
			1.0f, 0.0f,
			1.0f, 1.0f
		};
		br_RenderInfo& inf = mp_shape_info[shape];
		initBuffers(inf, sizeof(vertices), vertices, GL_TRIANGLE_STRIP, 4);
	}
	else if (shape == br_Shape::circle || shape == br_Shape::circle_empty)
	{
		std::vector<float> vertices;
		if (shape == br_Shape::circle) {
			vertices.push_back(0.5f);
			vertices.push_back(0.5f);
		}
		float precision = 360.0f;
		float step = 360.0f / precision;		
		for (float i = 0; i < 360.0f; i += step)
		{
			float angle = glm::radians(i);
			float x = (glm::cos(angle) + 1.0f) / 2.0f;
			float y = (glm::sin(angle) + 1.0f) / 2.0f;
			vertices.push_back(x);
			vertices.push_back(y);
		}
		vertices.push_back(vertices[2]);
		vertices.push_back(vertices[3]);
		br_RenderInfo& inf = mp_shape_info[shape];
		if (shape == br_Shape::circle)
			initBuffers(inf, vertices.size() * sizeof(float), vertices.data(), GL_TRIANGLE_FAN, (int)vertices.size() / 2);
		else
			initBuffers(inf, vertices.size() * sizeof(float), vertices.data(), GL_LINE_LOOP, (int)vertices.size() / 2);
	}	
	else if (shape == br_Shape::rectangle)
	{
		float vertices[] = {
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f
		};
		br_RenderInfo& inf = mp_shape_info[shape];
		initBuffers(inf, sizeof(vertices), vertices, GL_LINE_LOOP, 4);
	}
}
void BasicRenderer::RenderClosedPolygon(const std::vector<glm::vec2>& points, glm::vec2 position, glm::vec2 scale_points, glm::vec3 color)
{
	// Create VBO and copy data to it.
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), points.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glm::mat4 model(1.0f);	
	model = glm::translate(model, glm::vec3(position, 0.0f));	
	model = glm::scale(model, glm::vec3(scale_points, 0.0f));

	this->shader.Use().SetMat4("model", TileCamera2D::GetViewMatrix() * model);
	this->shader.SetVec3f("color", color);

	glDrawArrays(GL_LINE_LOOP, 0, (int)points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}
void BasicRenderer::RenderClosedPolygon(const std::vector<glm::vec2>& points, glm::mat4 customModel, glm::vec3 color)
{
	// Create VBO and copy data to it.
	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec2), points.data(), GL_DYNAMIC_DRAW);

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	this->shader.Use().SetMat4("model", TileCamera2D::GetViewMatrix() * customModel);
	this->shader.SetVec3f("color", color);

	glDrawArrays(GL_LINE_LOOP, 0, (int)points.size());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}
