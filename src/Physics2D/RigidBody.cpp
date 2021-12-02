#include "RigidBody.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "PhysicsWorld.h"

using namespace Physics2D;

std::string RigidBody::LastError = "";

RigidBody::RigidBody(glm::vec2 position, float density, float mass, float restitution, float area, bool isStatic, std::shared_ptr<Collider> collider)
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
	p.FrictionCoeff = 0.0f;
	this->Properties = p;
	this->isStatic = isStatic;
	this->collider = collider;
	this->position = this->collider->GetCenter();
	this->updateVerticesRequired = false;
	this->updateAABBRequired = false;
	

	UpdateColliderVertices();
	collider->UpdateAABB();
}
void RigidBody::SetUnitConverter(MeterUnitConverter* converter)
{
	this->UnitConverter = converter;
	this->collider->SetUnitConverter(converter);
}
bool RigidBody::checkAreaAndDensity(float& area, float& density, std::string& out_err)
{

	if (area < PhysicsWorld::GetMinBodySize())
	{
		out_err = "Circle radius is too small. Min circle area is " + std::to_string(PhysicsWorld::GetMinBodySize()) + ".";
		return false;
	}
	if (area > PhysicsWorld::GetMaxBodySize())
	{
		out_err = "Circle radius is too large. Max circle area is " + std::to_string(PhysicsWorld::GetMaxBodySize()) + ".";
		return false;
	}
	if (density < PhysicsWorld::GetMinDensity())
	{
		out_err = "Density is too small. Min density is " + std::to_string(PhysicsWorld::GetMinDensity()) + ".";
		return false;
	}
	if (density > PhysicsWorld::GetMaxDensity())
	{
		out_err = "Density is too large. Max density is " + std::to_string(PhysicsWorld::GetMaxDensity()) + ".";
		return false;
	}
	return true;
}
std::shared_ptr<RigidBody> RigidBody::CreateCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, std::string& out_errorMsg)
{
	float area = radius * radius * PI;

	if (!checkAreaAndDensity(area, density, out_errorMsg))
		return nullptr;

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	// Collider will be eventually deleted by the shared_ptr inside RigidBody class.
	std::shared_ptr<Collider> collider = std::shared_ptr<Collider>(new CircleCollider(position, radius));

	// mass = area * depth * density
	float mass = area * 1.0f * density;
	auto body = std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
	return body;
}
std::shared_ptr<RigidBody> RigidBody::CreateRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, std::string& out_errorMsg)
{
	float area = size.x * size.y;

	if (!checkAreaAndDensity(area, density, out_errorMsg))
		return nullptr;

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	// Collider will be eventually deleted by the shared_ptr inside RigidBody class.
	std::shared_ptr<Collider> collider = std::shared_ptr<Collider>(new RectangleCollider(position, size));

	// mass = area * depth * density
	float mass = area * 1.0f * density;
	auto body = std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
	return body;
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

	if (!checkAreaAndDensity(area, density, out_errorMsg))
		return nullptr;

	restitution = glm::clamp(restitution, 0.0f, 1.0f);

	std::shared_ptr<Collider> collider = std::shared_ptr<Collider>(new PolygonCollider(vertices));
	float mass = area * 1.0f * density;
	
	auto body = std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
	body->MoveTo(position);

	return body;
}
std::shared_ptr<RigidBody> RigidBody::CreateCapsuleBody(CapsuleOrientation o, glm::vec2 position, glm::vec2 size, float density, float isStatic, float restitution, std::string& out_errorMsg)
{
	float area = 0.0f;
	if (o == CapsuleOrientation::vertical)
		area = size.x * (size.y - size.x) + PI * size.x * size.x * 0.25f;
	else
		area = (size.x - size.y) * size.y + PI * size.y * size.y * 0.25f;

	if (!checkAreaAndDensity(area, density, out_errorMsg))
		return nullptr;

	float mass = area * 1.0f * density;

	std::shared_ptr<Collider> collider(new CapsuleCollider(o, position, position + size));
	return std::shared_ptr<RigidBody>(new RigidBody(position, density, mass, restitution, area, isStatic, collider));
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
std::shared_ptr<RigidBody> RigidBody::CreateCapsuleBody(CapsuleOrientation o, glm::vec2 position, glm::vec2 size, float density, float isStatic, float restitution)
{
	return CreateCapsuleBody(o, position, size, density, isStatic, restitution, LastError);
}

void RigidBody::UpdateColliderVertices()
{
	collider->SetCenter(position);
	glm::mat3 model = GetModel_m3();
	collider->UpdateVertices(model);
}
glm::mat3 RigidBody::GetModel_m3() 
{
	float sinA = glm::sin(rotation);
	float cosA = glm::cos(rotation);
	glm::mat3 model(1.0f);
	model[0] = glm::vec3(cosA, sinA, 0.0f);
	model[1] = glm::vec3(-sinA, cosA, 0.0f);
	model[2] = glm::vec3(position.x, position.y, 1.0f);
	return model;
}
glm::mat4 RigidBody::GetModel()
{
	glm::mat4 model(1.0f);
	model = glm::translate(model, glm::vec3(position, 0.0f));
	model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));

	return model;
}

void RigidBody::ApplyForces(float& dt, glm::vec2& gravity)
{
	if (IsKinematic)
		return;
	
	// F = ma
	// a = F / m
	Acceleration = (-LinearDrag * LinearVelocity + force) / Properties.Mass + this->GravityScale * gravity;

	this->LinearVelocity += Acceleration * dt;
	this->LinearVelocity += impulses; 

	this->position += LinearVelocity * dt;
	this->rotation += RotationalVelocity * dt;

	force = impulses = glm::vec2(0.0f, 0.0f);
	bIsDirty = true;
}
void RigidBody::UpdatePosition()
{
	if (LinearVelocity != glm::vec2(0.0f) || RotationalVelocity != 0.0f || updateAABBRequired || updateVerticesRequired)
	{
		UpdateColliderVertices();
		collider->UpdateAABB();
		bIsDirty = true;
	}
}
void RigidBody::Update(float dt, glm::vec2 gravity, int iterations) 
{
	if (isStatic)
		return;

	dt /= iterations;
	ApplyForces(dt, gravity);
	UpdatePosition();
}
void RigidBody::MoveOutOfCollision(const CollisionInfo& info)
{
	this->position += info.normal * info.depth;
	UpdateColliderVertices();
	collider->UpdateAABB();
	bIsDirty = true;
}