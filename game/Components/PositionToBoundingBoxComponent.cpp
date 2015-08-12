#include "pch.h"
#include "Components/PositionToBoundingBoxComponent.h"
#include "Core/Globals.h"

PositionToBoundingBoxComponent::PositionToBoundingBoxComponent()
	: _hash(0)
	, _entityChangeEvent(EntityEvents::ID_POSITION_TO_BOUNDING_BOX_CHANGED)
{
	Set(ff::INVALID_ENTITY, ff::RectFixedInt::Zeros());
}

void PositionToBoundingBoxComponent::Set(ff::Entity entity, ff::RectFixedInt rect)
{
	ff::hash_t oldHash = _hash;

	_points[0].SetPoint(rect.left, rect.top);
	_points[1].SetPoint(rect.right, rect.top);
	_points[2].SetPoint(rect.right, rect.bottom);
	_points[3].SetPoint(rect.left, rect.bottom);

	_hash = ff::HashFunc(rect);

	if (entity != ff::INVALID_ENTITY && _hash != oldHash)
	{
		entity->TriggerEvent(_entityChangeEvent);
	}
}
