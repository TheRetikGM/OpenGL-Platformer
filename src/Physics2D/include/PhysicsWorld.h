#pragma once
#include <vector>
#include "RigidBody.h"
#include "MeterUnitConverter.hpp"

/*
* This is my implementation of 2D physics engine.
* You can have multiple physics worlds which each handle its own bodies.
* Everything is in meters. That includes size of bodies etc.
* * For conversion you can define your units-per-meter ratio and then use functions ToUnits() and ToMeters() for conversions.
*/

namespace Physics2D
{
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

		void		AddBody(std::shared_ptr<Physics2D::RigidBody> body) { if (body) { body->UnitConverter = this; bodies.push_back(body); } }
		RigidBody*	AddCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution);
		RigidBody*	AddRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution);
		RigidBody*	AddPolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution);

		bool		RemoveBody(int index);
		int			GetBodyIndex(RigidBody* body);
		void		RemoveAllBodies() { bodies.clear(); }
		RigidBody*	GetBody(int index);
		std::string GetBodyName(int index);
		RigidBody*	GetNamedBody(std::string name);
		int			BodyCount() { return (int)bodies.size(); }
		void		Update(float dt, int precision = 1);

		glm::vec2	GetBodyAABBPosition(int index, bool inUnits = true) { return inUnits ? ToUnits(GetBody(index)->GetAABBPosition()) : GetBody(index)->GetAABBPosition(); }
		glm::vec2	GetBodyAABBSize(int index, bool inUnits = true) { return inUnits ? ToUnits(GetBody(index)->GetAABBSize()) : GetBody(index)->GetAABBSize(); }
		glm::vec2	GetBodyPosition(int index, bool inUnits = true) { return inUnits ? ToUnits(GetBody(index)->GetCenter()) : GetBody(index)->GetCenter(); }
		glm::vec2	GeNamedBodyAABBPosition(std::string name, bool inUnits = true) { return inUnits ? ToUnits(GetNamedBody(name)->GetAABBPosition()) : GetNamedBody(name)->GetAABBPosition(); }
		glm::vec2	GeNamedBodyAABBSize(std::string name, bool inUnits = true) { return inUnits ? ToUnits(GetNamedBody(name)->GetAABBSize()) : GetNamedBody(name)->GetAABBSize(); }
		glm::vec2	GeNamedBodyPosition(std::string name, bool inUnits = true) { return inUnits ? ToUnits(GetNamedBody(name)->GetCenter()) : GetNamedBody(name)->GetCenter(); }

	protected:
		static float minBodySize;
		static float maxBodySize;
		static float minDensity;
		static float maxDensity;
		static int minPrecision;
		static int maxPrecision;

		std::vector<std::shared_ptr<RigidBody>> bodies;
		// float meterUnitRatio;

		bool collideAABB(RigidBody* A, RigidBody* B);
		bool collide(RigidBody* b1, RigidBody* b2, CollisionInfo& info);
		void responseToCollision(RigidBody* b1, RigidBody* b2, const CollisionInfo& info);
		void callCollisionEnter_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
		void callCollisionExit_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info);
	};
}