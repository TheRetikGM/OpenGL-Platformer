#include "PhysicsWorld.h"
#include <map>
#include <algorithm>
#include <exception>
#include <stdexcept>

using namespace Physics2D;

float Physics2D::PhysicsWorld::minBodySize = 0.1f * 0.1f;
float Physics2D::PhysicsWorld::maxBodySize = 128.0f * 128.0f;
float Physics2D::PhysicsWorld::minDensity = 0.5f;
float Physics2D::PhysicsWorld::maxDensity = 21.4f;
int	  Physics2D::PhysicsWorld::minPrecision = 1;
int	  Physics2D::PhysicsWorld::maxPrecision = 128;

void PhysicsWorld::AddBody(std::shared_ptr<Physics2D::RigidBody> body) 
{ 
	if (body) 
	{ 
		body->SetUnitConverter(this);
		bodies.push_back(body);
		sortedBodies.push_back(new sw_aabb(body.get()));
	}
}
RigidBody* PhysicsWorld::AddCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution, bool inUnits)
{
	if (inUnits) {
		position = ToMeters(position);
		radius = ToMeters(radius);
	}

	std::string err;
	auto body = RigidBody::CreateCircleBody(position, radius, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	RigidBody* b = body.get();
	return b;
}
RigidBody* PhysicsWorld::AddRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution, bool inUnits)
{
	if (inUnits) {
		position = ToMeters(position);
		size = ToMeters(size);
	}

	std::string err;
	auto body = RigidBody::CreateRectangleBody(position, size, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	RigidBody* b = body.get();
	return b;
}
RigidBody* PhysicsWorld::AddPolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution, bool inUnits)
{
	if (inUnits) {
		for (glm::vec2& vertex : vertices)
		{
			vertex = ToMeters(vertex);
		}
		position = ToMeters(position);
	}

	std::string err;
	auto body = RigidBody::CreatePolygonBody(position, vertices, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	RigidBody* b = body.get();
	return b;
}
RigidBody* PhysicsWorld::AddCapsuleBody(CapsuleOrientation o, glm::vec2 position, glm::vec2 size, float density, float isStatic, float restitution, bool inUnits)
{
	if (inUnits) {
		position = ToMeters(position);
		size = ToMeters(size);
	}

	std::string err;
	auto body = RigidBody::CreateCapsuleBody(o, position, size, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	RigidBody* b = body.get();
	return b;
}

bool PhysicsWorld::RemoveBody(int index)
{
	if (index >= 0 && index < bodies.size())
	{
		// sortedBodies.remove_if([&](auto body) { return body.get() == bodies[index].get(); });
		for (int i = 0; i < sortedBodies.size(); i++) {
			if (sortedBodies[i]->body == bodies[index].get()) {
				delete sortedBodies[i];
				sortedBodies.erase(sortedBodies.begin() + i);
				break;
			}
		}
		bodies.erase(bodies.begin() + index);		

		return true;
	}
	return false;
}
void PhysicsWorld::RemoveAllBodies()
{
	bodies.clear();
	for (auto& i : sortedBodies)
		delete i;
	sortedBodies.clear();
}
RigidBody* PhysicsWorld::GetBody(int index)
{
	if (index < 0 || index >= bodies.size())
		return nullptr;
	return bodies[index].get();
}
int	PhysicsWorld::GetBodyIndex(RigidBody* body)
{
	for (int i = 0; i < bodies.size(); i++)
		if (bodies[i].get() == body)
			return i;
	return -1;
}
std::string PhysicsWorld::GetBodyName(int index)
{
	auto body = GetBody(index);
	return body ? body->Name : "null";
}
RigidBody* PhysicsWorld::GetNamedBody(std::string name)
{
	auto body_iter = std::find_if(bodies.begin(), bodies.end(), [&](std::shared_ptr<RigidBody> body) { return body->Name == name; });
	if (body_iter == bodies.end())
		return nullptr;
	return (*body_iter).get();
}

// Equasions used are from https://www.chrishecker.com/images/e/e7/Gdmphys3.pdf and https://box2d.org/files/ErinCatto_SequentialImpulses_GDC2006.pdf.
void PhysicsWorld::responseToCollision(RigidBody* b1, RigidBody* b2, const CollisionInfo& info, float& dt)
{
	auto b1_props = b1->GetPhysicsProperties();
	auto b2_props = b2->GetPhysicsProperties();
	
	// Realive velocity for two non-rotating bodies.
	glm::vec2 relativeVelocity = b1->LinearVelocity - b2->LinearVelocity;
	/* Compute and apply impulse */
	if (glm::dot(relativeVelocity, info.normal) > 0.0f)
		return;

	// Extra oomph
	float slop = info.depth / 3.0f;
	float bias_factor = 0.12f;
	float bias_velocity = (bias_factor / dt) * std::max(0.0f, info.depth - slop);

	// Compute normal impulse.
	float e = std::min(b1_props.Restitution, b2_props.Restitution);
	float j = -(1 + e) * glm::dot(relativeVelocity, info.normal) + bias_velocity;
	j /= (b1_props.invMass + b2_props.invMass);

	// Apply normal impulse to simulate realistic response.
	b1->LinearVelocity += j * b1_props.invMass * info.normal;
	b2->LinearVelocity -= j * b2_props.invMass * info.normal;

	// Compute friction tangent impulse.
	float friction = std::max(b1->Properties.FrictionCoeff, b2->Properties.FrictionCoeff);
	glm::vec2 t = glm::vec2(info.normal.y, -info.normal.x);
	float j_f = -glm::dot(relativeVelocity, t);
	j_f /= b1_props.invMass + b2_props.invMass;
	j_f = std::clamp(j_f, -friction * j, friction * j);

	// Apply tangent impulse to simulate friction.
	b1->LinearVelocity += j_f * b1_props.invMass * t;
	b2->LinearVelocity -= j_f * b2_props.invMass * t;
}

void PhysicsWorld::checkAndResolveCollision(RigidBody* bodyA, RigidBody* bodyB, float& dt)
{
	// If Axis-aligned bouding boxes aren't colliding then the two object cannot collide.
	if ((bodyA->IsStatic() && bodyB->IsStatic()) || !CheckCollision(bodyA->GetAABB(), bodyB->GetAABB()))
		return;

	CollisionInfo collisionInfo;
	if (CheckCollision(bodyA, bodyB, collisionInfo))
	{
		callCollisionEnter_callbacks(bodyA, bodyB, collisionInfo);

		if (bodyA->IsKinematic || bodyB->IsKinematic)
			return;

		// Move the bodies apart ( resolve collision ) based on their IsStatic property.
		/// If both bodies are static, then dont resolve the collision.
		if (bodyA->IsStatic() && bodyB->IsStatic())
			return;
		else if (bodyA->IsStatic() && !bodyB->IsStatic())
			bodyB->Move(-collisionInfo.normal * collisionInfo.depth);
		else if (!bodyA->IsStatic() && bodyB->IsStatic())
			bodyA->Move(collisionInfo.normal * collisionInfo.depth);
		else {
			bodyA->Move(collisionInfo.normal * collisionInfo.depth / 2.0f);
			bodyB->Move(-collisionInfo.normal * collisionInfo.depth / 2.0f);
		}


		// Apply realistic response of bodies to collision.
		responseToCollision(bodyA, bodyB, collisionInfo, dt);
		callCollisionExit_callbacks(bodyA, bodyB, collisionInfo);
	}
}

void PhysicsWorld::Update(float dt, int precision)
{
	precision = glm::clamp(precision, PhysicsWorld::minPrecision, PhysicsWorld::maxPrecision);
	float part_dt = dt / precision;

	for (int pre = 0; pre < precision; pre++)
	{
		// Move step.
		for (auto& body : bodies)
			body->Update(dt, Gravity, precision);

		// Collision step
		std::sort(sortedBodies.begin(), sortedBodies.end(), [&](const sw_aabb* A, const sw_aabb* B) {
			return (*A->position)[sortAxis] < (*B->position)[sortAxis];
		});

		glm::vec2 s(0.0f), s2(0.0f), v(0.0f);
		for (auto i = sortedBodies.begin(); i != sortedBodies.end(); i++)
		{
			// Compute center of AABB
			glm::vec2 p = 0.5f * ((*i)->Min() + *(*i)->size);
			s += p;
			s2 += p * p;

			for (auto j = std::next(i); j != sortedBodies.end(); j++)
			{
				// Check if object A max is less then object B min.
				if ((*i)->Max()[sortAxis] < (*j)->Min()[sortAxis])
					break;
				checkAndResolveCollision((*i)->body, (*j)->body, part_dt);
			}
		}

		// Select x or y based on avarage variance of object centers.
		v = s2 - s * s / float(sortedBodies.size());
		sortAxis = 0;
		if (v.y > v.x) sortAxis = 1;

		// Move step.
		for (auto& body : bodies)
			body->UpdatePosition();
	}
}
void PhysicsWorld::callCollisionEnter_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info)
{
	A->OnCollisionEnter(B, info);
	CollisionInfo i = info;
	i.normal = -i.normal;
	B->OnCollisionEnter(A, i);
	OnCollisionEnter(A, B, info);
}
void PhysicsWorld::callCollisionExit_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info)
{
	A->OnCollisionExit(B, info);
	CollisionInfo i = info;
	i.normal = -i.normal;
	B->OnCollisionExit(A, i);
	OnCollisionExit(A, B, info);
}
