#pragma once

struct VelocityComponent;

struct PositionComponent : public ff::Component
{
	PositionComponent();

	void Set(ff::Entity entity, ff::PointFixedInt pos);
	void Set(ff::Entity entity, ff::PointFixedInt pos, ff::FixedInt scale, ff::FixedInt rotate);
	void Offset(ff::Entity entity, ff::PointFixedInt pos);
	void Offset(ff::Entity entity, ff::FixedInt x, ff::FixedInt y);
	void Update(ff::Entity entity, const VelocityComponent& vel);

	ff::PointFixedInt _pos;
	ff::FixedInt _scale;
	ff::FixedInt _rotate; // degrees
	ff::hash_t _hash;
};
