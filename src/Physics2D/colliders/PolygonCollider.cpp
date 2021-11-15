#include "colliders/PolygonCollider.h"

using namespace Physics2D;

void GetPolygonAABB(std::vector<glm::vec2>& vertices, AABB& o_aabb)
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
glm::vec2 GetPolygonCenter(std::vector<glm::vec2>& vertices)
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

void PolygonCollider::UpdateAABB() {
	GetPolygonAABB(this->Vertices, this->aabb);
}
PolygonCollider::PolygonCollider(std::vector<glm::vec2> vertices)
	: Collider({ 0.0f, 0.0f }, { 0.0f, 0.0f })
{
	this->Vertices = vertices;
	this->UnitVertices = vertices;
	UpdateAABB();
	this->center = GetPolygonCenter(this->Vertices);

	for (auto& unitVertex : this->UnitVertices)
		unitVertex -= this->center;

	type = ColliderType::polygon;
}