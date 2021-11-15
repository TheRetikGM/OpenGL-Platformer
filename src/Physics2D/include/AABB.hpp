#pragma once
#include "MeterUnitConverter.hpp"
#include <glm/glm.hpp>

namespace Physics2D
{
	struct AABB {
		MeterUnitConverter* UnitConverter = nullptr;
		glm::vec2 position;
		glm::vec2 size;

		AABB(glm::vec2 position, glm::vec2 size) : position(position), size(size) {}

		static AABB CreateFromMinMax(glm::vec2 min, glm::vec2 max) { return AABB(min, max - min); }
		/*static AABB CreateForCircle(glm::vec2 center, float radius) { return AABB(center - glm::vec2(); }*/

		void SetPosition(glm::vec2 pos) { position = pos; }
		void SetSize(glm::vec2 size) { this->size = size; }
		glm::vec2 GetPosition(bool inUnits = false) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(position) : position; }
		glm::vec2 GetSize(bool inUnits = false) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(size) : size; }
		float GetLeft() const { return position.x; }
		float GetRight() const { return position.x + size.x; }
		float GetTop() const { return position.y; }
		float GetBottom() const { return position.y + size.y; }
		glm::vec2 GetMin() const { return position; }
		glm::vec2 GetMax() const { return position + size; }
		glm::vec2 GetCenter() const { return position + 0.5f * size; }

		void SetPosition(glm::vec2 pos, bool inUnits) { if (inUnits) pos = UnitConverter->ToMeters(pos); position = pos; }
		void SetSize(glm::vec2 size, bool inUnits) { if (inUnits) size = UnitConverter->ToMeters(size); this->size = size; }
		float GetLeft(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetLeft()) : GetLeft(); }
		float GetRight(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetRight()) : GetRight(); }
		float GetTop(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetTop()) : GetTop(); }
		float GetBottom(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetBottom()) : GetBottom(); }
		glm::vec2 GetMin(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetMin()) : GetMin(); }
		glm::vec2 GetMax(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetMax()) : GetMax(); }
		glm::vec2 GetCenter(bool inUnits) const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(GetCenter()) : GetCenter(); }
	};

	inline bool CheckCollision(const AABB& a, const AABB& b)
	{
		return a.GetLeft() < b.GetRight()
			&& a.GetRight() > b.GetLeft()
			&& a.GetTop() < b.GetBottom()
			&& a.GetBottom() > b.GetTop();
	}
}