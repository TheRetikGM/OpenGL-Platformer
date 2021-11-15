#pragma once
#include "colliders/Collider.h"

namespace Physics2D
{
	class RectangleCollider : public Collider
	{
	public:
		RectangleCollider(glm::vec2 pos, glm::vec2 size);

		void UpdateAABB() override;
		glm::vec2 GetRectSize() const { return rectSize; }
		glm::vec2 GetRectSize(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(rectSize) : rectSize; }

	protected:
		glm::vec2 rectSize;
	};
};