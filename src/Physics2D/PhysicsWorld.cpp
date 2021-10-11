#include "PhysicsWorld.h"
#include <map>
#include <algorithm>
#include <exception>
#include <stdexcept>

using namespace Physics2D;

float Physics2D::PhysicsWorld::minBodySize = 0.1f * 0.1f;
float Physics2D::PhysicsWorld::maxBodySize = 64.0f * 64.0f;
float Physics2D::PhysicsWorld::minDensity = 0.5f;
float Physics2D::PhysicsWorld::maxDensity = 21.4f;
int	  Physics2D::PhysicsWorld::minPrecision = 1;
int	  Physics2D::PhysicsWorld::maxPrecision = 128;

RigidBody* PhysicsWorld::AddCircleBody(glm::vec2 position, float radius, float density, bool isStatic, float restitution)
{
	std::string err;
	auto body = RigidBody::CreateCircleBody(position, radius, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	return body.get();
}
RigidBody* PhysicsWorld::AddRectangleBody(glm::vec2 position, glm::vec2 size, float density, bool isStatic, float restitution)
{
	std::string err;
	auto body = RigidBody::CreateRectangleBody(position, size, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	return body.get();
}
RigidBody* PhysicsWorld::AddPolygonBody(glm::vec2 position, std::vector<glm::vec2> vertices, float density, bool isStatic, float restitution)
{
	std::string err;
	auto body = RigidBody::CreatePolygonBody(position, vertices, density, isStatic, restitution, err);
	if (!body)
		throw std::runtime_error(err.c_str());
	AddBody(body);
	return body.get();
}

bool PhysicsWorld::RemoveBody(int index)
{
	if (index >= 0 && index < bodies.size())
	{
		bodies.erase(bodies.begin() + index);
		return true;
	}
	return false;
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
bool PhysicsWorld::collide(RigidBody* b1, RigidBody* b2, CollisionInfo& info)
{
	return Physics2D::CheckCollision(b1->GetCollider(), b2->GetCollider(), info);
}
void PhysicsWorld::Update(float dt, int precision)
{
	precision = glm::clamp(precision, PhysicsWorld::minPrecision, PhysicsWorld::maxPrecision);
	
	for (int pre = 0; pre < precision; pre++)
	{
		// Movement step
		for (auto& body : bodies) {
			body->Update(dt, Gravity, precision);
			body->IsColliding = false;
		}

		// Collision step
		for (int i = 0; i < bodies.size(); i++)
		{
			for (int j = i + 1; j < bodies.size(); j++)
			{
				auto bodyA = bodies[i].get();
				auto bodyB = bodies[j].get();

				// If Axis-aligned bouding boxes aren't colliding then the two object cannot collide.
				if ((bodyA->GetIsStatic() && bodyB->GetIsStatic()) || !collideAABB(bodyA, bodyB))
					continue;

				CollisionInfo collisionInfo;
				if (collide(bodyA, bodyB, collisionInfo))
				{
					callCollisionEnter_callbacks(bodyA, bodyB, collisionInfo);

					// Move the bodies apart ( resolve collision ) based on their IsStatic property.
					/// If both bodies are static, then dont resolve the collision.
					if (bodyA->GetIsStatic() && bodyB->GetIsStatic())
						continue;
					else if (bodyA->GetIsStatic() && !bodyB->GetIsStatic())
						bodyB->Move(collisionInfo.normal * collisionInfo.depth);
					else if (!bodyA->GetIsStatic() && bodyB->GetIsStatic())
						bodyA->Move(-collisionInfo.normal * collisionInfo.depth);
					else {
						bodyA->Move(-collisionInfo.normal * collisionInfo.depth / 2.0f);
						bodyB->Move(collisionInfo.normal * collisionInfo.depth / 2.0f);
					}

					callCollisionExit_callbacks(bodyA, bodyB, collisionInfo);

					// Apply realistic response of bodies to collision.
					responseToCollision(bodyA, bodyB, collisionInfo);
				}
			}
		}
	}
}

/// Equasions used are from https://www.chrishecker.com/images/e/e7/Gdmphys3.pdf.
void PhysicsWorld::responseToCollision(RigidBody* b1, RigidBody* b2, const CollisionInfo& info)
{
	auto b1_props = b1->GetPhysicsProperties();
	auto b2_props = b2->GetPhysicsProperties();
	glm::vec2 relativeVelocity = b2->LinearVelocity - b1->LinearVelocity;

	if (glm::dot(relativeVelocity, info.normal) > 0.0f)
		return;

	float e = std::min(b1_props.Restitution, b2_props.Restitution);
	float j = -(1.0f + e) * glm::dot(relativeVelocity, info.normal);
	j /= b1_props.invMass + b2_props.invMass;

	glm::vec2 impulse = j * info.normal;

	b1->LinearVelocity -= impulse * b1_props.invMass;
	b2->LinearVelocity += impulse * b2_props.invMass;
}

// Basic AABB collision check for the two bodies.
bool PhysicsWorld::collideAABB(RigidBody* A, RigidBody* B)
{
	Physics2D::Collider* a = A->GetCollider();
	Physics2D::Collider* b = B->GetCollider();
	return a->GetLeft() < b->GetRight()
		&& a->GetRight() > b->GetLeft()
		&& a->GetTop() < b->GetBottom()
		&& a->GetBottom() > b->GetTop();
}

void PhysicsWorld::callCollisionEnter_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info)
{
	A->OnCollisionEnter(B, info);
	B->OnCollisionEnter(A, info);
	OnCollisionEnter(A, B, info);
}
void PhysicsWorld::callCollisionExit_callbacks(RigidBody* A, RigidBody* B, const CollisionInfo& info)
{
	A->OnCollisionExit(B, info);
	B->OnCollisionExit(A, info);
	OnCollisionExit(A, B, info);
}
