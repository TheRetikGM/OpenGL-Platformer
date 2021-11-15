#pragma once
#include "colliders/Collider.h"

namespace Physics2D
{
	class PolygonCollider : public Collider
	{
	public:
		PolygonCollider(std::vector<glm::vec2> vertices);

		void UpdateAABB() override;
	protected:
	};
}