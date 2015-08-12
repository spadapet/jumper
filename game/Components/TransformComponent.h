#pragma once

struct PositionComponent;

struct TransformComponent : public ff::Component
{
	TransformComponent();

	void Set(ff::Entity entity, const PositionComponent& pos);
	DirectX::XMMATRIX GetTransform() const;
	DirectX::XMMATRIX GetInverseTransform() const;
	const DirectX::XMFLOAT4X4& GetTransformRaw() const;
	const DirectX::XMFLOAT4X4& GetInverseTransformRaw() const;

	ff::PointFixedInt TransformFromWorld(ff::PointFixedInt pos) const;
	ff::PointFixedInt TransformToWorld(ff::PointFixedInt pos) const;
	void TransformFromWorld(const ff::PointFixedInt* pos, size_t count, ff::PointFixedInt* outPos) const;
	void TransformToWorld(const ff::PointFixedInt* pos, size_t count, ff::PointFixedInt* outPos) const;

	DirectX::XMFLOAT4X4 _transform;
	DirectX::XMFLOAT4X4 _inverseTransform;
	ff::hash_t _hashComputedInverse;
	ff::hash_t _hash;
};
