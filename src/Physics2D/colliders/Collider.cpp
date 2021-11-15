#include "colliders/Collider.h"
#include <vector>
#include <functional>

using namespace Physics2D;
using glm::vec2;
using std::vector;

void Collider::SetUnitConverter(MeterUnitConverter* converter)
{
	this->UnitConverter = converter;
	this->aabb.UnitConverter = converter;
}
void Collider::UpdateVertices(glm::mat3 model)
{
	for (int i = 0; i < (int)UnitVertices.size(); i++)
	{
		glm::vec3 t = model * glm::vec3(UnitVertices[i], 1.0f);
		Vertices[i].x = t.x;
		Vertices[i].y = t.y;
	}
}

