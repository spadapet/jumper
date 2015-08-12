#pragma once

struct PositionToBoundingBoxComponent;
struct TransformComponent;

struct BoundingBoxComponent : public ff::Component
{
	BoundingBoxComponent();

	void Set(ff::Entity entity, const ff::PointFixedInt* fourPoints);
	void Set(ff::Entity entity, const PositionToBoundingBoxComponent& posToBox, const TransformComponent& transform);

	ff::RectFixedInt _box; // axis aligned around the transformed points
	ff::PointFixedInt _points[4]; // tl, tr, br, bl
	ff::hash_t _hash;
	ff::hash_t _entityChangeEvent;
};
