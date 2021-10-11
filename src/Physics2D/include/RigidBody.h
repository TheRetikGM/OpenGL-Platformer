#pragma once
#include <cmath>
#include <memory>
#include <glm/glm.hpp>
#include <exception>
#include <stdexcept>
#include <string>
#include <functional>
#include <any>

#include "Colliders.h"
#include "MeterUnitConverter.hpp"
#define M_PI       3.14159265358979323846   // pi

namespace Physics2D
{
	const float PI = (float)M_PI;

	struct PhysicsProperties
	{
		float Density;
		float Mass;
		float Restitution;	// Bounciness
		float Area;
		float invMass;
		float FrictionCoefficient;
	};

	class RigidBody
	{
	public:		
		bool		IsColliding = false;
		glm::vec2	LinearVelocity;
		float		RotationalVelocity;
		float		GravityScale;
		PhysicsProperties Properties;


		std::string Name = "";
		std::any	CustomProperties = std::any("");

		MeterUnitConverter* UnitConverter;

		// Events
		std::function<void(RigidBody*, const CollisionInfo&)> OnCollisionEnter = [](RigidBody* body, const CollisionInfo& info) {};
		std::function<void(RigidBody*, const CollisionInfo&)> OnCollisionExit =  [](RigidBody* body, const CollisionInfo& info) {};

		PhysicsProperties GetPhysicsProperties() const { return Properties; }
		bool		 GetIsStatic()		const { return isStatic; }
		Collider*	 GetCollider()		const { return collider.get(); }
		glm::vec2	 GetAABBSize(bool inUnits = false)		const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(collider->GetSize()) : collider->GetSize(); }
		glm::vec2	 GetCenter(bool inUnits = false)		const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(collider->GetCenter()) : collider->GetCenter(); }
		ColliderType GetType()								const { return collider->GetType(); }
		glm::vec2	 GetAABBPosition(bool inUnits = false)	const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(collider->GetPosition()) : collider->GetPosition(); }
		glm::vec2	 GetPosition(bool inUnits = false)	const { return (inUnits && UnitConverter) ? UnitConverter->ToUnits(this->position) : this->position; }
		float		 GetRotation()		const { return rotation; }
		float		 GetRotVelocity()	const { return RotationalVelocity; }

		void Move(glm::vec2 amount, bool inUnits = false) { 
			this->position += (inUnits && UnitConverter) ? UnitConverter->ToMeters(amount) : amount;
			updateVerticesRequired = updateAABBRequired = true;
		}
		void MoveTo(glm::vec2 pos, bool inUnits = false) {
			this->position = (inUnits && UnitConverter) ? UnitConverter->ToMeters(pos) : pos;
			updateVerticesRequired = updateAABBRequired = true;
		}
		void Rotate(float angle) { 
			this->rotation += angle;
			updateVerticesRequired = updateAABBRequired = true;
		}
		void AddForce(glm::vec2 force) {
			this->force += force;			
		}
		glm::vec2 GetForce() { return force; }
		void Update(float dt, glm::vec2 gravity = glm::vec2(0.0f, 9.81), int iterations = 1);

		void UpdateColliderVertices();
		glm::mat4 GetModel();

		static std::string LastError;
		static std::shared_ptr<RigidBody> CreateCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, std::string& out_errorMsg);
		static std::shared_ptr<RigidBody> CreateRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, std::string& out_errorMsg);
		static std::shared_ptr<RigidBody> CreatePolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution, std::string& out_errorMsg);
		static std::shared_ptr<RigidBody> CreateCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution);
		static std::shared_ptr<RigidBody> CreateRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution);
		static std::shared_ptr<RigidBody> CreatePolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution);

	protected:
		glm::vec2 position;
		float rotation;
		glm::vec2 force;

		bool isStatic;
		std::shared_ptr<Collider> collider;

		bool updateVerticesRequired;
		bool updateAABBRequired;

		float meterUnitRatio = 1;
		float invMeterUnitRatio = 1;

		RigidBody(glm::vec2 position, float density, float mass, float restitution, float area, bool isStatic, Collider* collider);
	};

}