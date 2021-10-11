#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <cstring>
#include <vector>

namespace Physics2D
{
	struct CollisionInfo {
		glm::vec2 normal;
		float depth;

		CollisionInfo(glm::vec2 norm, float depth) : normal(norm), depth(depth) {}
		CollisionInfo() : normal(glm::vec2(0.0f)), depth(0.0f) {}
	};

	enum class ColliderType : int {
		rectangle = 0,
		circle = 1,
		polygon = 2
	};

	class Collider
	{
	public:
		// Shape vertices
		std::vector<glm::vec2> Vertices;
		// Original vertices with center at [0,0]
		std::vector<glm::vec2> UnitVertices;

		virtual glm::vec2 GetCenter() const { return center; }
		void SetCenter(glm::vec2 value) { center = value; }
		virtual void UpdateAABBProperties() {}
		virtual void onVerticesUpdated() {}

		glm::vec2 GetPosition() const { return position; }
		ColliderType GetType() const { return type; }
		glm::vec2 GetSize() const { return size; }
		float GetLeft() const { return position.x; }
		float GetRight() const { return position.x + size.x; }
		float GetTop() const { return position.y; }
		float GetBottom() const { return position.y + size.y; }
		template<class T> T Get() { return dynamic_cast<T>(this); }

	protected:
		// AABB properties
		glm::vec2 position;
		glm::vec2 size;

		glm::vec2 center;

		ColliderType type;

		Collider(glm::vec2 pos, glm::vec2 size) 
			: size(size), type(ColliderType::rectangle), position(pos), Vertices(), center(pos + size / 2.0f), UnitVertices() {
		}
	};

	class RectangleCollider : public Collider
	{
	public:
		RectangleCollider(glm::vec2 pos, glm::vec2 size);

		void UpdateAABBProperties() override;
		glm::vec2 GetRectSize() { return rectSize; }

	protected:
		glm::vec2 rectSize;
	};

	class CircleCollider : public Collider
	{
	public:
		CircleCollider(glm::vec2 centerPos, float radius) : Collider(centerPos - glm::vec2(radius), glm::vec2(radius * 2.0f)), radius(radius) {
			type = ColliderType::circle;
			Vertices.push_back(centerPos);
			UnitVertices.push_back({ 0.0f, 0.0f });
		}

		float GetRadius() const { return radius; }
		void UpdateAABBProperties() override {
			position = Vertices[0] - glm::vec2(radius, radius);
			size = glm::vec2(radius, radius) * 2.0f;
		};
 
	protected:
		float radius;
	};

	class PolygonCollider : public Collider
	{
	public:
		PolygonCollider(std::vector<glm::vec2> vertices);

		void UpdateAABBProperties() override;
	protected:
	};

	bool CheckCollision(const CircleCollider* c1, const CircleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const RectangleCollider* c1, const RectangleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const RectangleCollider* c1, const CircleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const PolygonCollider* c1, const PolygonCollider* c2, CollisionInfo& info);
	bool CheckCollision(const PolygonCollider* c1, const CircleCollider* c2, CollisionInfo& info);
	bool CheckCollision(const PolygonCollider* c1, const RectangleCollider* c2, CollisionInfo& info);
	bool CheckCollision(Collider* c1, Collider* c2, CollisionInfo& info);
}