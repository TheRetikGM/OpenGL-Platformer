#pragma once
#include "colliders/Collider.h"
#include "colliders/CircleCollider.h"
#include "colliders/RectangleCollider.h"
#include "colliders/PolygonCollider.h"
#include "colliders/CapsuleCollider.h"
#define ReverseCheckCollision(A, B, info)\
	bool _ret = CheckCollision(B, A, info);\
	info.normal = -info.normal;\
	return _ret;

namespace Physics2D
{
	struct CollisionInfo {
		glm::vec2 normal;
		float depth;

		CollisionInfo(glm::vec2 norm, float depth) : normal(norm), depth(depth) {}
		CollisionInfo() : normal(glm::vec2(0.0f)), depth(0.0f) {}
	};

	bool CheckCollision(const CapsuleCollider* A, const CapsuleCollider* B, CollisionInfo& info);
	bool CheckCollision(const CapsuleCollider* A, const CircleCollider* B, CollisionInfo& info);
	bool CheckCollision(const CapsuleCollider* A, const RectangleCollider* B, CollisionInfo& info);
	bool CheckCollision(const CapsuleCollider* A, const PolygonCollider* B, CollisionInfo& info);

	bool CheckCollision(const CircleCollider* c1, const CircleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const CircleCollider* c1, const CapsuleCollider* c2, CollisionInfo& info); // Reverse
	bool CheckCollision(const CircleCollider* c1, const RectangleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const CircleCollider* c1, const PolygonCollider* c2, CollisionInfo& info);

	bool CheckCollision(const RectangleCollider* c1, const RectangleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const RectangleCollider* c1, const CapsuleCollider* c2, CollisionInfo& info); // Reverse
	bool CheckCollision(const RectangleCollider* c1, const CircleCollider* c2, CollisionInfo& info);  // Reverse
	bool CheckCollision(const RectangleCollider* c1, const PolygonCollider* c2, CollisionInfo& info);

	bool CheckCollision(const PolygonCollider* c1, const PolygonCollider* c2, CollisionInfo& info);
	bool CheckCollision(const PolygonCollider* c1, const CapsuleCollider* c2, CollisionInfo& info);	 // Reverse
	bool CheckCollision(const PolygonCollider* c1, const CircleCollider* c2, CollisionInfo& info);	 // Reverse
	bool CheckCollision(const PolygonCollider* c1, const RectangleCollider* c2, CollisionInfo& info);// Reverse

	bool CheckCollision(Collider* c1, Collider* c2, CollisionInfo& info);
}