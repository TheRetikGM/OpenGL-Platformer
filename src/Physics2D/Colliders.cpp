#include "Colliders.h"
#include <vector>

using namespace Physics2D;
using glm::vec2;
using std::vector;

bool Physics2D::CheckCollision(const CircleCollider* A, const CircleCollider* B, glm::vec2& out_normal, float& out_depth)
{
	float distance = glm::length(B->GetCenter() - A->GetCenter());
	float min_distance = A->GetRadius() + B->GetRadius();
	if (distance < min_distance)
	{
		out_normal = glm::normalize(B->GetCenter() - A->GetCenter());
		out_depth = min_distance - distance;
		return true;
	}
	out_normal = glm::vec2(0.0f);
	out_depth = 0.0f;

	return false;
}

glm::vec2 GetCenter(std::vector<glm::vec2>& p)
{
	glm::vec2 center(0.0f);
	for (auto& i : p)
		center += i;
	center /= p.size();
	return center;
}

void ProjectVertices(std::vector<glm::vec2>& v, glm::vec2 axis, float& o_min, float& o_max)
{
	o_min = INFINITY;
	o_max = -INFINITY;
	for (int p = 0; p < v.size(); p++)
	{
		float q = glm::dot(v[p], axis);
		o_min = std::min(o_min, q);
		o_max = std::max(o_max, q);
	}
}
bool SAT_CollisionCheck(std::vector<glm::vec2>& p1, std::vector<glm::vec2>& p2, glm::vec2& out_normal, float& out_depth)
{
	vector<vec2>* poly1 = &p1;
	vector<vec2>* poly2 = &p2;

	out_depth = INFINITY;
	out_normal = glm::vec2(0.0f);

	for (int shape = 0; shape < 2; shape++)
	{
		if (shape == 1)
		{
			poly1 = &p2;
			poly2 = &p1;
		}

		for (int a = 0; a < poly1->size(); a++)
		{
			int b = (a + 1) % (int)poly1->size();
			vec2 edge = (*poly1)[b] - (*poly1)[a];
			vec2 axisProj = glm::normalize(glm::vec2(-edge.y, edge.x));

			float min_r1, max_r1;
			ProjectVertices(*poly1, axisProj, min_r1, max_r1);
			float min_r2, max_r2;
			ProjectVertices(*poly2, axisProj, min_r2, max_r2);

			if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
				return false;

			float overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
			if (overlap < out_depth)
			{
				out_depth = overlap;
				out_normal = axisProj;
			}
		}
	}

	glm::vec2 center1 = GetCenter(p1);
	glm::vec2 center2 = GetCenter(p2);
	if (glm::dot(out_normal, glm::normalize(center2 - center1)) < 0.0f)
		out_normal = -out_normal;

	return true;
}
void ProjectCircle(glm::vec2& circleCenter, float& circleRadius, glm::vec2 axis, float& o_min, float& o_max)
{
	axis = glm::normalize(axis);
	glm::vec2 v1 = circleCenter - circleRadius * axis;
	glm::vec2 v2 = circleCenter + circleRadius * axis;
	o_min = glm::dot(axis, v1);
	o_max = glm::dot(axis, v2);

	if (o_min > o_max)
		std::swap(o_min, o_max);
}
glm::vec2 GetClosestPointToCircle(glm::vec2& circleCenter, std::vector<glm::vec2>& v)
{
	float d = INFINITY;
	glm::vec2 closestPoint(0.0f, 0.0f);
	for (auto& i : v)
	{
		float dist = glm::length(i - circleCenter);
		if (dist < d)
		{
			d = dist;
			closestPoint = i;
		}
	}
	return closestPoint;
}
bool SAT_CollisionCheck(glm::vec2 circleCenter, float circleRadius, std::vector<glm::vec2> p, glm::vec2& out_normal, float& out_depth)
{
	vector<vec2>* poly1 = &p;

	out_depth = INFINITY;
	out_normal = glm::vec2(0.0f);

	for (int a = 0; a < poly1->size() + 1; a++)
	{
		vec2 axisProj(0.0f);
		if (a != poly1->size())
		{
			int b = (a + 1) % (int)poly1->size();
			vec2 edge = (*poly1)[b] - (*poly1)[a];
			axisProj = glm::normalize(glm::vec2(-edge.y, edge.x));
		}
		else {
			glm::vec2 CenterToVert = GetClosestPointToCircle(circleCenter, p) - circleCenter;			
			axisProj = glm::normalize(CenterToVert);
		}

		float min_r1, max_r1;
		ProjectVertices(*poly1, axisProj, min_r1, max_r1);
		float min_r2, max_r2;
		ProjectCircle(circleCenter, circleRadius, axisProj, min_r2, max_r2);

		if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
			return false;

		float overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
		if (overlap < out_depth)
		{
			out_depth = overlap;
			out_normal = axisProj;
		}
	}

	glm::vec2 center1 = GetCenter(p);
	glm::vec2 center2 = circleCenter;
	if (glm::dot(out_normal, glm::normalize(center2 - center1)) < 0.0f)
		out_normal = -out_normal;

	return true;
}

bool Physics2D::CheckCollision(const RectangleCollider* c1, const RectangleCollider* c2, glm::vec2& out_normal, float& out_depth)
{
	auto p1 = c1->Vertices;
	auto p2 = c2->Vertices;
	return SAT_CollisionCheck(p1, p2, out_normal, out_depth);
}
bool Physics2D::CheckCollision(const RectangleCollider* c1, const CircleCollider* c2, glm::vec2& out_normal, float& out_depth)
{	
	return SAT_CollisionCheck(c2->GetCenter(), c2->GetRadius(), (std::vector<glm::vec2>)c1->Vertices, out_normal, out_depth);
}
bool Physics2D::CheckCollision(const PolygonCollider* c1, const PolygonCollider* c2, glm::vec2& out_normal, float& out_depth)
{
	auto p1 = c1->Vertices;
	auto p2 = c2->Vertices;
	return SAT_CollisionCheck(p1, p2, out_normal, out_depth);
}
bool Physics2D::CheckCollision(const PolygonCollider* c1, const CircleCollider* c2, glm::vec2& out_normal, float& out_depth)
{
	auto p1 = c1->Vertices;
	return SAT_CollisionCheck(c2->GetCenter(), c2->GetRadius(), p1, out_normal, out_depth);
}
bool Physics2D::CheckCollision(const PolygonCollider* c1, const RectangleCollider* c2, glm::vec2& out_normal, float& out_depth)
{
	auto p1 = c1->Vertices;
	auto p2 = c2->Vertices;
	return SAT_CollisionCheck(p1, p2, out_normal, out_depth);
}

bool Physics2D::CheckCollision(Collider* c1, Collider* c2, glm::vec2& out_normal, float& out_depth)
{
	auto t1 = c1->GetType();
	auto t2 = c2->GetType();

	if (t1 == ColliderType::circle && t2 == ColliderType::circle)
		return CheckCollision(c1->Get<CircleCollider*>(), c2->Get<CircleCollider*>(), out_normal, out_depth);
	else if (t1 == ColliderType::rectangle && t2 == ColliderType::rectangle)
		return CheckCollision(c1->Get<RectangleCollider*>(), c2->Get<RectangleCollider*>(), out_normal, out_depth);
	else if (t1 == ColliderType::rectangle && t2 == ColliderType::circle)
		return CheckCollision(c1->Get<RectangleCollider*>(), c2->Get<CircleCollider*>(), out_normal, out_depth);
	else if (t1 == ColliderType::circle && t2 == ColliderType::rectangle) {
		bool result = CheckCollision(c2->Get<RectangleCollider*>(), c1->Get<CircleCollider*>(), out_normal, out_depth);
		out_normal = -out_normal;
		return result;
	}
	else if (t1 == ColliderType::polygon && t2 == ColliderType::polygon)
		return CheckCollision(c1->Get<PolygonCollider*>(), c2->Get<PolygonCollider*>(), out_normal, out_depth);
	else if (t1 == ColliderType::polygon && t2 == ColliderType::rectangle)
		return CheckCollision(c1->Get<PolygonCollider*>(), c2->Get<RectangleCollider*>(), out_normal, out_depth);
	else if (t1 == ColliderType::polygon && t2 == ColliderType::circle)
		return CheckCollision(c1->Get<PolygonCollider*>(), c2->Get<CircleCollider*>(), out_normal, out_depth);		
	else if (t1 == ColliderType::rectangle && t2 == ColliderType::polygon) {
		auto result = CheckCollision(c2->Get<PolygonCollider*>(), c1->Get<RectangleCollider*>(), out_normal, out_depth);
		out_normal = -out_normal;
		return result;
	}
	else if (t1 == ColliderType::circle && t2 == ColliderType::polygon) {
		auto result = CheckCollision(c2->Get<PolygonCollider*>(), c1->Get<CircleCollider*>(), out_normal, out_depth);
		out_normal = -out_normal;
		return result;
	}
	return false;
}


void GetPolygonAABB(std::vector<glm::vec2>& vertices, glm::vec2& o_pos, glm::vec2& o_size)
{
	o_pos = glm::vec2(INFINITY, INFINITY);
	o_size = glm::vec2(-INFINITY, -INFINITY);
	for (auto& i : vertices)
	{
		o_pos.x = std::min(o_pos.x, i.x);
		o_pos.y = std::min(o_pos.y, i.y);
		o_size.x = std::max(o_size.x, i.x);
		o_size.y = std::max(o_size.y, i.y);
	}
	o_size -= o_pos;
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

/* Rectangle Collider */
RectangleCollider::RectangleCollider(glm::vec2 pos, glm::vec2 size) : Collider(pos, size), rectSize(size) {
	Vertices.reserve(4 * 2);
	Vertices.push_back({ pos.x, pos.y });
	Vertices.push_back({ pos.x + size.x, pos.y });
	Vertices.push_back({ pos.x + size.x, pos.y + size.y });
	Vertices.push_back({ pos.x, pos.y + size.y });

	center = GetPolygonCenter(Vertices);
	for (int i = 0; i < Vertices.size(); i++)
		UnitVertices.push_back(Vertices[i] - center);
}
void RectangleCollider::UpdateAABBProperties() {
	GetPolygonAABB(this->Vertices, this->position, this->size);
}

/* Polygon Collider */
void PolygonCollider::UpdateAABBProperties() {
	GetPolygonAABB(this->Vertices, this->position, this->size);
}
PolygonCollider::PolygonCollider(std::vector<glm::vec2> vertices)
	: Collider({ 0.0f, 0.0f }, { 0.0f, 0.0f })
{
	this->Vertices = vertices;
	this->UnitVertices = vertices;
	UpdateAABBProperties();
	this->center = GetPolygonCenter(this->Vertices);

	for (auto& unitVertex : this->UnitVertices)
		unitVertex -= this->center;

	type = ColliderType::polygon;
}