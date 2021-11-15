#include "colliders/RectangleCollider.h"

using namespace Physics2D;

glm::vec2 GetRectangleCenter(std::vector<glm::vec2>& vertices)
{
	glm::vec2 sum_center(0.0f, 0.0f);
	float sum_weight = 0.0f;
	for (int a = 0; a < vertices.size(); a++)
	{
		int b = (a + 1) % (int)vertices.size();
		int c = (a + 2) % (int)vertices.size();
		float weight = glm::length(vertices[b] - vertices[a]) + glm::length(vertices[b] - vertices[c]);
		sum_center += vertices[b] * weight;
		sum_weight += weight;
	}

	return sum_center / sum_weight;
}

RectangleCollider::RectangleCollider(glm::vec2 pos, glm::vec2 size) : Collider(pos, size), rectSize(size) {
	Vertices.reserve(4 * 2);
	Vertices.push_back({ pos.x, pos.y });
	Vertices.push_back({ pos.x + size.x, pos.y });
	Vertices.push_back({ pos.x + size.x, pos.y + size.y });
	Vertices.push_back({ pos.x, pos.y + size.y });

	center = GetRectangleCenter(Vertices);
	for (int i = 0; i < Vertices.size(); i++)
		UnitVertices.push_back(Vertices[i] - center);
}
void GetRectangleAABB(std::vector<glm::vec2>& vertices, AABB& o_aabb)
{
	o_aabb.position = glm::vec2(INFINITY, INFINITY);
	o_aabb.size = glm::vec2(-INFINITY, -INFINITY);
	for (auto& i : vertices)
	{
		o_aabb.position.x = std::min(o_aabb.position.x, i.x);
		o_aabb.position.y = std::min(o_aabb.position.y, i.y);
		o_aabb.size.x = std::max(o_aabb.size.x, i.x);
		o_aabb.size.y = std::max(o_aabb.size.y, i.y);
	}
	o_aabb.size -= o_aabb.position;
}
void RectangleCollider::UpdateAABB() {
	GetRectangleAABB(this->Vertices, this->aabb);
}