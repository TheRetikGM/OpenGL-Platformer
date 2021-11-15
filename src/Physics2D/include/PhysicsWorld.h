#pragma once
#include <vector>
#include "AABB.hpp"
#include "RigidBody.h"
#include "MeterUnitConverter.hpp"
#include "helper.hpp"
#include <list>

/*
* This is my implementation of 2D physics engine.
* You can have multiple physics worlds which each handle its own bodies.
* Everything is in meters. That includes size of bodies etc.
* * For conversion you can define your units-per-meter ratio and then use functions ToUnits() and ToMeters() for conversions.
*/

namespace Physics2D
{
	// Struct for Sort & Sweep algorithm. Sorting is significantly faster when using this struct.
	struct sw_aabb
	{
		const glm::vec2* position;
		const glm::vec2* size;
		RigidBody* body;

		sw_aabb(RigidBody* b) : position(&b->GetAABB().position), size(&b->GetAABB().size), body(b) {}

		const glm::vec2& Min() const { return *position; }
		const glm::vec2 Max() const { return *position + *size; }
	};

	class PhysicsWorld : public MeterUnitConverter
	{
	public:
		// Definition of gravity.
		glm::vec2 Gravity;

		/* Events */
		std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> OnCollisionEnter = [](RigidBody* A, RigidBody* B, const CollisionInfo& info) {};
		std::function<void(RigidBody*, RigidBody*, const CollisionInfo&)> OnCollisionExit = [](RigidBody* A, RigidBody* B, const CollisionInfo& info) {};

		static float GetMinBodySize() { return minBodySize; }
		static float GetMaxBodySize() { return maxBodySize; }
		static float GetMinDensity() { return minDensity; }
		static float GetMaxDensity() { return maxDensity; }
		static int GetMinPrecision() { return minPrecision; }
		static int GetMaxPrecision() { return maxPrecision; }

		PhysicsWorld(float units_per_meter = 1.0f) : bodies(), Gravity(0.0f, 9.81f), MeterUnitConverter(1.0f / units_per_meter) {}

		void		AddBody(std::shared_ptr<Physics2D::RigidBody> body);
		RigidBody*	AddCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*	AddRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*	AddPolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution, bool inUnits = false);
		RigidBody*	AddCapsuleBody(CapsuleOrientation o, glm::vec2 position, glm::vec2 size, float density, float isStatic, float restitution, bool inUnits = false);

		bool		RemoveBody(int index);
		int			GetBodyIndex(RigidBody* body);
		void		RemoveAllBodies();
		RigidBody*	GetBody(int index);
		std::string GetBodyName(int index);
		RigidBody*	GetNamedBody(std::string name);
		int			BodyCount() { return (int)bodies.size(); }
		void		Update(float dt, int precision = 1);

		const AABB& GetBodyAABB(int index) { return GetBody(index)->GetAABB(); }
		glm::vec2	GetBodyPosition(int index, bool inUnits = true) { return GetBody(index)->GetCenter(inUnits); }
		const AABB& GeNamedBodyAABB(std::string name) { return GetNamedBody(name)->GetAABB(); }
		glm::vec2	GeNamedBodyPosition(std::string name, bool inUnits = true) { return GetNamedBody(name)->GetCenter(inUnits); }


		// Sort and Sweep variables.
		int sortAxis = 0;	// Either 0 (x axis) or 1 (y axis)
	protected:
		static float minBodySize;
		static float maxBodySize;
		static float minDensity;
		static float maxDensity;
		static int minPrecision;
		static int maxPrecision;

		// Array of bodies.
		std::vector<std::shared_ptr<RigidBody>> bodies;
		// Array used for Sort & Sweep algorithm.
		std::vector<sw_aabb*> sortedBodies;

		void checkAndResolveCollision(RigidBody* bodyA, RigidBody* bodyB, float& dt);
		void responseToCollision(RigidBody* b1, RigidBody* b2, const CollisionInfo& info, float& dt);
		void callCollisionEnter_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
		void callCollisionExit_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
	};
}