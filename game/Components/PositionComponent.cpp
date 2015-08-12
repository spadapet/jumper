#include "pch.h"
#include "Components/PositionComponent.h"
#include "Components/VelocityComponent.h"
#include "Core/Globals.h"

PositionComponent::PositionComponent()
	: _hash(0)
{
	Set(ff::INVALID_ENTITY, ff::PointFixedInt::Zeros(), 1_f, 0_f);
}

void PositionComponent::Set(ff::Entity entity, ff::PointFixedInt pos)
{
	Set(entity, pos, _scale, _rotate);
}

void PositionComponent::Set(ff::Entity entity, ff::PointFixedInt pos, ff::FixedInt scale, ff::FixedInt rotate)
{
	ff::hash_t oldHash = _hash;

	_pos = pos;
	_scale = scale;
	_rotate = rotate;
	_hash = ff::HashBytes(this, sizeof(ff::FixedInt) * 4);

	if (entity != ff::INVALID_ENTITY && _hash != oldHash)
	{
		entity->TriggerEvent(EntityEvents::ID_POSITION_CHANGED);
	}
}

void PositionComponent::Offset(ff::Entity entity, ff::PointFixedInt pos)
{
	Set(entity, _pos + pos);
}

void PositionComponent::Offset(ff::Entity entity, ff::FixedInt x, ff::FixedInt y)
{
	Offset(entity, ff::PointFixedInt(x, y));
}

void PositionComponent::Update(ff::Entity entity, const VelocityComponent& vel)
{
	ff::FixedInt rotate = _rotate;

	if (vel._rotateVelocity != 0_f)
	{
		rotate += vel._rotateVelocity;

		while (rotate < 0_f)
		{
			rotate += 360_f;
		}

		while (rotate > 360_f)
		{
			rotate -= 360_f;
		}
	}

	Set(entity, _pos + vel._posVelocity, _scale, rotate);
}
