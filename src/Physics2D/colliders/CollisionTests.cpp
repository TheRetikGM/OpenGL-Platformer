#include "colliders/CollisionTests.h"
#include <vector>

#define CapsuleChecks(A, B, info)\
		CollisionInfo i[3];\
		info.depth = INFINITY;\
		memset(i, 0, 3 * sizeof(CollisionInfo));\
		\
		bool c1 = CheckCollision(A->TopCollider.get(), B, i[0]);\
		bool c2 = CheckCollision(A->MiddleCollider.get(), B, i[1]);\
		bool c3 = CheckCollision(A->BottomCollider.get(), B, i[2]);\
		\
		glm::vec2 centersVec = A->BottomCollider->UnitVertices[0] - A->TopCollider->UnitVertices[0];\
		\
		if (c1 && glm::dot(centersVec, i[0].normal) > 0.0f)\
			info = i[0];\
		if (c2 && info.depth > i[1].depth)\
			info = i[1];\
		if (c3 && glm::dot(centersVec, i[2].normal) < 0.0f && info.depth > i[2].depth)\
			info = i[2];\
		\
		if (info.depth == INFINITY)\
			info.depth = 0.0f;\
		\
		return c1 || c2 || c3;\

using namespace Physics2D;

void ClipPoints(std::vector<glm::vec2>& o_clippedPoints, glm::vec2 p1, glm::vec2 p2, glm::vec2 n, float o)
{
	o_clippedPoints.clear();
	float d1 = glm::dot(n, p1) - o;
	float d2 = glm::dot(n, p2) - o;

	if (d1 >= 0.0f) o_clippedPoints.push_back(p1);
	if (d2 >= 0.0f) o_clippedPoints.push_back(p2);

	if (d1 * d2 < 0.0f)
	{
		glm::vec2 e = p2 - p1;
		float u = d1 / (d1 - d2);
		e *= u;
		e += p1;
		o_clippedPoints.push_back(e);
	}
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
bool SAT_CollisionCheck(std::vector<glm::vec2>& p1, std::vector<glm::vec2>& p2, CollisionInfo& info)
{
	std::vector<glm::vec2>* poly1 = &p1;
	std::vector<glm::vec2>* poly2 = &p2;

	info.depth = INFINITY;
	info.normal = glm::vec2(0.0f);

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
			glm::vec2 edge = (*poly1)[b] - (*poly1)[a];
			glm::vec2 axisProj = glm::normalize(glm::vec2(-edge.y, edge.x));

			float min_r1, max_r1;
			ProjectVertices(*poly1, axisProj, min_r1, max_r1);
			float min_r2, max_r2;
			ProjectVertices(*poly2, axisProj, min_r2, max_r2);

			if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
				return false;

			float overlap = std::min(max_r1, max_r2) - std::max(min_r1, min_r2);
			if (overlap < info.depth)
			{
				info.depth = overlap;
				info.normal = -axisProj;
			}
		}
	}

	glm::vec2 center1 = GetCenter(p1);
	glm::vec2 center2 = GetCenter(p2);
	if (glm::dot(info.normal, glm::normalize(center1 - center2)) < 0.0f)
		info.normal = -info.normal;

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
bool SAT_CollisionCheck(glm::vec2 circleCenter, float circleRadius, std::vector<glm::vec2> p, CollisionInfo& info)
{
	std::vector<glm::vec2>* poly1 = &p;

	info.depth = INFINITY;
	info.normal = glm::vec2(0.0f);

	for (int a = 0; a < poly1->size() + 1; a++)
	{
		glm::vec2 axisProj(0.0f);
		if (a != poly1->size())
		{
			int b = (a + 1) % (int)poly1->size();
			glm::vec2 edge = (*poly1)[b] - (*poly1)[a];
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
		if (overlap < info.depth)
		{
			info.depth = overlap;
			info.normal = -axisProj;
		}
	}

	glm::vec2 center1 = GetCenter(p);
	glm::vec2 center2 = circleCenter;
	if (glm::dot(info.normal, glm::normalize(center2 - center1)) < 0.0f)
		info.normal = -info.normal;

	return true;
}

bool Physics2D::CheckCollision(const CapsuleCollider* A, const CapsuleCollider* B, CollisionInfo& info)
{
	CapsuleChecks(A, B, info);
}
bool Physics2D::CheckCollision(const CapsuleCollider* A, const CircleCollider* B, CollisionInfo& info)
{
	CapsuleChecks(A, B, info);
}
bool Physics2D::CheckCollision(const CapsuleCollider* A, const RectangleCollider* B, CollisionInfo& info)
{
	CapsuleChecks(A, B, info);
}
bool Physics2D::CheckCollision(const CapsuleCollider* A, const PolygonCollider* B, CollisionInfo& info)
{
	CapsuleChecks(A, B, info);
}

bool Physics2D::CheckCollision(const CircleCollider* A, const CircleCollider* B, CollisionInfo& info)
{
	float distance = glm::length(B->GetCenter() - A->GetCenter());
	float min_distance = A->GetRadius() + B->GetRadius();
	if (distance < min_distance)
	{
		info.normal = glm::normalize(A->GetCenter() - B->GetCenter());
		info.depth = min_distance - distance;
		return true;
	}
	info.normal = glm::vec2(0.0f);
	info.depth = 0.0f;

	return false;
}
bool Physics2D::CheckCollision(const CircleCollider* c1, const RectangleCollider* c2, CollisionInfo& info)
{
	return SAT_CollisionCheck(c1->GetCenter(), c1->GetRadius(), (std::vector<glm::vec2>)c2->Vertices, info);
}
bool Physics2D::CheckCollision(const CircleCollider* c1, const PolygonCollider* c2, CollisionInfo& info)
{
	return SAT_CollisionCheck(c1->GetCenter(), c1->GetRadius(), (std::vector<glm::vec2>)c2->Vertices, info);
}
bool Physics2D::CheckCollision(const RectangleCollider* c1, const RectangleCollider* c2, CollisionInfo& info)
{
	return SAT_CollisionCheck((std::vector<glm::vec2>)c1->Vertices, (std::vector<glm::vec2>)c2->Vertices, info);
}
bool Physics2D::CheckCollision(const RectangleCollider* c1, const PolygonCollider* c2, CollisionInfo& info)
{
	return SAT_CollisionCheck((std::vector<glm::vec2>)c1->Vertices, (std::vector<glm::vec2>)c2->Vertices, info);
}
bool Physics2D::CheckCollision(const PolygonCollider* c1, const PolygonCollider* c2, CollisionInfo& info)
{
	return SAT_CollisionCheck((std::vector<glm::vec2>)c1->Vertices, (std::vector<glm::vec2>)c2->Vertices, info);
}

bool Physics2D::CheckCollision(const CircleCollider* c1, const CapsuleCollider* c2, CollisionInfo& info)	{ ReverseCheckCollision(c1, c2, info); }
bool Physics2D::CheckCollision(const RectangleCollider* c1, const CapsuleCollider* c2, CollisionInfo& info)	{ ReverseCheckCollision(c1, c2, info); }
bool Physics2D::CheckCollision(const RectangleCollider* c1, const CircleCollider* c2, CollisionInfo& info)	{ ReverseCheckCollision(c1, c2, info); }
bool Physics2D::CheckCollision(const PolygonCollider* c1, const CapsuleCollider* c2, CollisionInfo& info)	{ ReverseCheckCollision(c1, c2, info); }
bool Physics2D::CheckCollision(const PolygonCollider* c1, const CircleCollider* c2, CollisionInfo& info)	{ ReverseCheckCollision(c1, c2, info); }
bool Physics2D::CheckCollision(const PolygonCollider* c1, const RectangleCollider* c2, CollisionInfo& info) { ReverseCheckCollision(c1, c2, info); }

bool Physics2D::CheckCollision(Collider* A, Collider* B, CollisionInfo& info)
{
	auto t1 = A->GetType();
	auto t2 = B->GetType();

	switch (t1)
	{
	case ColliderType::circle:
		if (t2 == ColliderType::circle)
			return CheckCollision(A->Get<CircleCollider*>(), B->Get<CircleCollider*>(), info);
		else if (t2 == ColliderType::rectangle)
			return CheckCollision(A->Get<CircleCollider*>(), B->Get<RectangleCollider*>(), info);
		else if (t2 == ColliderType::polygon)
			return CheckCollision(A->Get<CircleCollider*>(), B->Get<PolygonCollider*>(), info);
		else if (t2 == ColliderType::capsule)
			return CheckCollision(A->Get<CircleCollider*>(), B->Get<CapsuleCollider*>(), info);
		break;
	case ColliderType::rectangle:
		if (t2 == ColliderType::circle)
			return CheckCollision(A->Get<RectangleCollider*>(), B->Get<CircleCollider*>(), info);
		else if (t2 == ColliderType::rectangle)
			return CheckCollision(A->Get<RectangleCollider*>(), B->Get<RectangleCollider*>(), info);
		else if (t2 == ColliderType::polygon)
			return CheckCollision(A->Get<RectangleCollider*>(), B->Get<PolygonCollider*>(), info);
		else if (t2 == ColliderType::capsule)
			return CheckCollision(A->Get<RectangleCollider*>(), B->Get<CapsuleCollider*>(), info);
		break;
	case ColliderType::polygon:
		if (t2 == ColliderType::circle)
			return CheckCollision(A->Get<PolygonCollider*>(), B->Get<CircleCollider*>(), info);
		else if (t2 == ColliderType::rectangle)
			return CheckCollision(A->Get<PolygonCollider*>(), B->Get<RectangleCollider*>(), info);
		else if (t2 == ColliderType::polygon)
			return CheckCollision(A->Get<PolygonCollider*>(), B->Get<PolygonCollider*>(), info);
		else if (t2 == ColliderType::capsule)
			return CheckCollision(A->Get<PolygonCollider*>(), B->Get<CapsuleCollider*>(), info);
		break;
	case ColliderType::capsule:
		if (t2 == ColliderType::circle)
			return CheckCollision(A->Get<CapsuleCollider*>(), B->Get<CircleCollider*>(), info);
		else if (t2 == ColliderType::rectangle)
			return CheckCollision(A->Get<CapsuleCollider*>(), B->Get<RectangleCollider*>(), info);
		else if (t2 == ColliderType::polygon)
			return CheckCollision(A->Get<CapsuleCollider*>(), B->Get<PolygonCollider*>(), info);
		else if (t2 == ColliderType::capsule)
			return CheckCollision(A->Get<CapsuleCollider*>(), B->Get<CapsuleCollider*>(), info);
		break;
	default:
		break;
	}
	return false;
}