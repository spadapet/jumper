#include "pch.h"
#include "Components/BoundingBoxComponent.h"
#include "Components/PositionToBoundingBoxComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"

BoundingBoxComponent::BoundingBoxComponent()
	: _box(ff::RectFixedInt::Zeros())
	, _entityChangeEvent(EntityEvents::ID_BOUNDING_BOX_CHANGED)
{
	ff::ZeroObject(_points);
	_hash = ff::HashFunc(_points);
}

void BoundingBoxComponent::Set(ff::Entity entity, const ff::PointFixedInt* fourPoints)
{
	ff::hash_t oldHash = _hash;

	std::memcpy(_points, fourPoints, sizeof(ff::PointFixedInt) * 4);
	_hash = ff::HashFunc(_points);

	ff::PointFixedInt minPoint = _points[0];
	ff::PointFixedInt maxPoint = _points[0];

	for (size_t i = 1; i < 4; i++)
	{
		minPoint.x = std::min(minPoint.x, _points[i].x);
		minPoint.y = std::min(minPoint.y, _points[i].y);
		maxPoint.x = std::max(maxPoint.x, _points[i].x);
		maxPoint.y = std::max(maxPoint.y, _points[i].y);
	}

	_box.SetRect(minPoint, maxPoint);

	if (entity != ff::INVALID_ENTITY && _hash != oldHash)
	{
		entity->TriggerEvent(_entityChangeEvent);
	}
}

void BoundingBoxComponent::Set(ff::Entity entity, const PositionToBoundingBoxComponent& posToBox, const TransformComponent& transform)
{
	ff::PointFixedInt points[4];
	transform.TransformFromWorld(posToBox._points, 4, points);
	Set(entity, points);
}
