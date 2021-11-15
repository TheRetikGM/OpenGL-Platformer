#pragma once
#include "colliders/Collider.h"

namespace Physics2D
{
	class CircleCollider : public Collider
	{
	public:
		CircleCollider(glm::vec2 centerPos, float radius) : Collider(centerPos - glm::vec2(radius), glm::vec2(radius * 2.0f)), radius(radius) {
			type = ColliderType::circle;
			Vertices.push_back(centerPos);
			UnitVertices.push_back({ 0.0f, 0.0f });
		}

		const float& GetRadius() const { return radius; }
		const float &GetRadius(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(radius) : radius; }
		void UpdateAABB() override {
			aabb.position = Vertices[0] - glm::vec2(radius, radius);
			aabb.size = glm::vec2(radius, radius) * 2.0f;
		};

	protected:
		float radius;
	};
};