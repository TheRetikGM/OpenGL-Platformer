#include "colliders/CapsuleCollider.h"
#include <exception>
#include <stdexcept>

using namespace Physics2D;

CapsuleCollider::CapsuleCollider(CapsuleOrientation o, glm::vec2 center_pos, float center_dist, float radius)
	: orientation(o)
	, Collider(glm::vec2(0.0f), glm::vec2(0.0f))
{
	if (center_dist > -0.0001f && center_dist < 0.0001f)
		throw std::runtime_error("CapsuleCollider::CapsuleCollider(): Cannot create capsule collider with center_dist == 0.");

	center = center_pos;
	this->center_dist = center_dist;
	this->radius = radius;
	this->init();
}
CapsuleCollider::CapsuleCollider(CapsuleOrientation o, glm::vec2 aabb_min, glm::vec2 aabb_max)
	: orientation(o), Collider(glm::vec2(0.0f), glm::vec2(0.0f))
{
	glm::vec2 size = aabb_max - aabb_min;
	if (o == CapsuleOrientation::vertical) {
		this->center_dist = (size.y - size.x) / 2.0f;
		this->radius = size.x / 2.0f;
	}
	else {
		this->center_dist = (size.x - size.y) / 2.0f;
		this->radius = size.y / 2.0f;
	}
	if (this->center_dist > -0.0001f && this->center_dist < 0.0001f)
		throw std::runtime_error("CapsuleCollider::CapsuleCollider(): Cannot create capsule collider with center_dist == 0.");
	center = aabb_min + size / 2.0f;
	init();
}
void CapsuleCollider::init()
{
	this->type = ColliderType::capsule;
	if (this->orientation == CapsuleOrientation::horizontal)
		initHCapsule();
	else
		initVCapsule();

	UpdateAABB();
}
void CapsuleCollider::initVCapsule()
{
	this->TopCollider = std::make_shared<CircleCollider>(center + glm::vec2(0.0f, -center_dist), radius);
	this->MiddleCollider = std::make_shared<RectangleCollider>(center + glm::vec2(-radius, -center_dist), glm::vec2(radius, center_dist) * 2.0f);
	this->BottomCollider = std::make_shared<CircleCollider>(center + glm::vec2(0.0f, center_dist), radius);

	this->TopCollider->UnitVertices[0] = glm::vec2(0.0f, -center_dist);
	this->BottomCollider->UnitVertices[0] = glm::vec2(0.0f, center_dist);
}
void CapsuleCollider::initHCapsule()
{
	this->TopCollider = std::make_shared<CircleCollider>(center + glm::vec2(-center_dist, 0.0f), radius);
	this->MiddleCollider = std::make_shared<RectangleCollider>(center + glm::vec2(-center_dist, -radius), glm::vec2(center_dist, radius) * 2.0f);
	this->BottomCollider = std::make_shared<CircleCollider>(center + glm::vec2(center_dist, 0.0f), radius);

	this->TopCollider->UnitVertices[0] = glm::vec2(-center_dist, 0.0f);
	this->BottomCollider->UnitVertices[0] = glm::vec2(center_dist, 0.0f);
}
void CapsuleCollider::UpdateAABB()
{
	TopCollider->UpdateAABB();
	MiddleCollider->UpdateAABB();
	BottomCollider->UpdateAABB();

	this->aabb.position = TopCollider->GetAABB().position;
	this->aabb.size = BottomCollider->GetAABB().GetMax() - TopCollider->GetAABB().position;
}
void CapsuleCollider::UpdateVertices(glm::mat3 model)
{
	TopCollider->UpdateVertices(model);
	TopCollider->SetCenter(center + TopCollider->UnitVertices[0]);

	MiddleCollider->SetCenter(center);
	MiddleCollider->UpdateVertices(model);

	BottomCollider->UpdateVertices(model);
	BottomCollider->SetCenter(center + BottomCollider->UnitVertices[0]);
}
void CapsuleCollider::SetUnitConverter(MeterUnitConverter* converter)
{
	this->UnitConverter = converter;
	this->aabb.UnitConverter = converter;
	this->TopCollider->SetUnitConverter(converter);
	this->MiddleCollider->SetUnitConverter(converter);
	this->BottomCollider->SetUnitConverter(converter);
}