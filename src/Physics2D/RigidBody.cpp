#include "RigidBody.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "PhysicsWorld.h"

using namespace Physics2D;

std::string RigidBody::LastError = "";

RigidBody::RigidBody(glm::vec2 position, float density, float mass, float restitution, float area, bool isStatic, Collider* collider)
{
	this->position = position;
	this->LinearVelocity = { 0.0f, 0.0f };
	this->rotation = 0.0f;
	this->RotationalVelocity = 0.0f;
	this->GravityScale = 1.0f;
	this->force = glm::vec2(0.0f, 0.0f);
	PhysicsProperties p;
	p.Density = density;
	p.Mass = mass;
	p.Restitution = restitution;
	p.Area = area;
	p.invMass = (isStatic) ? 0.0f : 1.0f / p.Mass;
	this->Properties = p;
	this->isStatic = isStatic;
	this->collider = std::shared_ptr<Collider>(collider);
	this->position = this->collider->GetCenter();
	this->updateVerticesRequired = false;
	this->updateAABBRequired = false;
	

	UpdateColliderVertices();
	collider->UpdateAABBProperties();
}

std::shared_ptr<RigidBody> RigidBody::CreateCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, std::string& out_errorMsg)
{
	float area = radius * radius * PI;

	if (area < PhysicsWorld::GetMinBodySize())
	{
		out_errorMsg = "Circle radius is too small. Min circle area is " + std::to_string(PhysicsWorld::GetMinBodySize()) + ".";
		return nullptr;
	}
	if (area > PhysicsWorld::GetMaxBodySize())
	{
		out_errorMsg = "Circle radius is too large. Max circle area is " + std::to_string(PhysicsWorld::GetMaxBodySize()) + ".";
		return nullptr;
	}
	if (density < PhysicsWorld::GetMinDensity())
	{
		out_errorMsg = "Density is too small. Min density is " + std::to_string(PhysicsWorld::GetMinDensity()) + ".";
		return nullptr;
	}
	if (density > PhysicsWorld::GetMaxDensity())
	{
		out_errorMsg = "Density is too large. Max density is " + std::to_string(PhysicsWorld::GetMaxDensity()) + ".";
		return nullptr;
	}

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	// Collider will be eventually deleted by the shared_ptr inside RigidBody class.
	Collider* collider = new CircleCollider(position, radius);

	// mass = area * depth * density
	float mass = area * 1.0f * density;
	
	return std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
}
std::shared_ptr<RigidBody> RigidBody::CreateRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, std::string& out_errorMsg)
{
	float area = size.x * size.y;

	if (area < PhysicsWorld::GetMinBodySize())
	{
		out_errorMsg = "Area is too small. Min rectangle area is " + std::to_string(PhysicsWorld::GetMinBodySize()) + ".";
		return nullptr;
	}
	if (area > PhysicsWorld::GetMaxBodySize())
	{
		out_errorMsg = "Area is too large. Max rectangle area is " + std::to_string(PhysicsWorld::GetMaxBodySize()) + ".";
		return nullptr;
	}
	if (density < PhysicsWorld::GetMinDensity())
	{
		out_errorMsg = "Density is too small. Min density is " + std::to_string(PhysicsWorld::GetMinDensity()) + ".";
		return nullptr;
	}
	if (density > PhysicsWorld::GetMaxDensity())
	{
		out_errorMsg = "Density is too large. Max density is " + std::to_string(PhysicsWorld::GetMaxDensity()) + ".";
		return nullptr;
	}

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	// Collider will be eventually deleted by the shared_ptr inside RigidBody class.
	Collider* collider = new RectangleCollider(position, size);

	// mass = area * depth * density
	float mass = area * 1.0f * density;
	
	return std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
}
std::shared_ptr<RigidBody> RigidBody::CreatePolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution, std::string& out_errorMsg)
{
	float area = 0.0f;

	for (int a = 0; a < vertices.size(); a++)
	{
		int b = (a + 1) % (int)vertices.size();
		area += vertices[a].x * vertices[b].y - vertices[a].y * vertices[b].x;
	}
	area = (1.0f / 2.0f) * area;

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	Collider* collider = new PolygonCollider(vertices);
	float mass = area * 1.0f * density;
	
	auto body = std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
	body->MoveTo(position);

	return body;
}
std::shared_ptr<RigidBody> RigidBody::CreateCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution)
{
	return CreateCircleBody(position, radius, density, isStatic, restitution, LastError);
}
std::shared_ptr<RigidBody> RigidBody::CreateRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution)
{
	return CreateRectangleBody(position, size, density, isStatic, restitution, LastError);
}
std::shared_ptr<RigidBody> RigidBody::CreatePolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution)
{
	return CreatePolygonBody(position, vertices, density, isStatic, restitution, LastError);
}

void RigidBody::UpdateColliderVertices()
{
	if (collider->UnitVertices.size() > 0)
	{
		float sinA = glm::sin(rotation);
		float cosA = glm::cos(rotation);		
		glm::mat4 model = GetModel();

		for (int i = 0; i < collider->UnitVertices.size(); i++)
			collider->Vertices[i] = glm::vec2(model * glm::vec4(collider->UnitVertices[i], 0.0f, 1.0f));
	}
	collider->SetCenter(position);
}
glm::mat4 RigidBody::GetModel()
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::rotate(model, -rotation, glm::vec3(0.0f, 0.0f, 1.0f));

	return model;
}

void RigidBody::Update(float dt, glm::vec2 gravity, int iterations) 
{
	if (isStatic)
		return;

	dt /= iterations;

	// force = acc * mass
	// acc = force / mass
	glm::vec2 acceleration = this->force / this->Properties.Mass;
	this->LinearVelocity += acceleration * dt;
	this->LinearVelocity += this->GravityScale * gravity * dt;

	this->position += LinearVelocity * dt;
	this->rotation += RotationalVelocity * dt;

	// Update AABB or rotate if needed.
	if (LinearVelocity != glm::vec2(0.0f) || RotationalVelocity != 0.0f || updateAABBRequired || updateVerticesRequired)
	{
		UpdateColliderVertices();
		collider->UpdateAABBProperties();
	}

	force = glm::vec2(0.0f, 0.0f);
}