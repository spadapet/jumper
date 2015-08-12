#pragma once

struct PositionToBoundingBoxComponent : public ff::Component
{
	PositionToBoundingBoxComponent();

	void Set(ff::Entity entity, ff::RectFixedInt rect);

	ff::PointFixedInt _points[4];
	ff::hash_t _hash;
	ff::hash_t _entityChangeEvent;
};
