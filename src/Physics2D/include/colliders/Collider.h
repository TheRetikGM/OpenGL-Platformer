#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <cstring>
#include <vector>
#include "MeterUnitConverter.hpp"
#include "AABB.hpp"

namespace Physics2D
{
	enum class ColliderType : int {
		rectangle = 0,
		circle = 1,
		polygon = 2,
		capsule = 3
	};

	class Collider
	{
	public:
		// Shape vertices
		std::vector<glm::vec2> Vertices;
		// Original vertices with center at [0,0]
		std::vector<glm::vec2> UnitVertices;
		MeterUnitConverter* UnitConverter = nullptr;

		virtual const glm::vec2& GetCenter() const { return center; }
		virtual glm::vec2 GetCenter(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetCenter()) : GetCenter(); }
		virtual void UpdateAABB() {}
		virtual void onVerticesUpdated() {}
		virtual void UpdateVertices(glm::mat3 model);
		virtual void SetUnitConverter(MeterUnitConverter* converter);
		void SetCenter(glm::vec2 value) { center = value; }
		void SetCenter(glm::vec2 value, bool inUnits) { center = (inUnits && UnitConverter) ? UnitConverter->ToMeters(value) : value; }

		ColliderType GetType() const { return type; }
		const AABB& GetAABB() const { return aabb; }
		template<class T> T Get() { return dynamic_cast<T>(this); }

	protected:
		// Center position.
		glm::vec2 center;
		// Axis aligned bounding box.
		AABB aabb;
		// The type of the collider.
		ColliderType type;

		Collider(glm::vec2 pos, glm::vec2 size)
			: aabb(pos, size), type(ColliderType::rectangle), Vertices(), center(pos + size / 2.0f), UnitVertices() {
		}
	};
}