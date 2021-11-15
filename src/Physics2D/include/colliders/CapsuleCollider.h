#pragma once
#include "colliders/Collider.h"
#include "colliders/CircleCollider.h"
#include "colliders/RectangleCollider.h"

namespace Physics2D
{
	enum class CapsuleOrientation : uint8_t { vertical, horizontal };

	class CapsuleCollider : public Collider
	{
	public:
		std::shared_ptr<CircleCollider> TopCollider;
		std::shared_ptr<RectangleCollider> MiddleCollider;
		std::shared_ptr<CircleCollider> BottomCollider;

		CapsuleCollider(CapsuleOrientation o, glm::vec2 center_pos, float center_dist, float radius);
		CapsuleCollider(CapsuleOrientation o, glm::vec2 aabb_min, glm::vec2 aabb_max);

		const float& GetRadius() const { return TopCollider->GetRadius(); }
		const float& GetCenterDist() const { return center_dist; }
		float GetRadius(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(radius) : radius; }
		float GetCenterDist(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(center_dist) : center_dist; }
		const glm::vec2 GetCentersVec() const { return BottomCollider->GetCenter() - TopCollider->GetCenter(); }
		const CapsuleOrientation& GetOrientation() const { return orientation; }
		void UpdateAABB() override;
		void UpdateVertices(glm::mat3 model) override;
		void SetUnitConverter(MeterUnitConverter* converter) override;

	protected:
		float radius;
		float center_dist;
		CapsuleOrientation	orientation;

		void init();
		void initHCapsule();
		void initVCapsule();
	};
}