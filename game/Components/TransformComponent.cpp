#include "pch.h"
#include "Components/PositionComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"

TransformComponent::TransformComponent()
{
	_transform = ff::GetIdentityMatrix();
	_inverseTransform = ff::GetIdentityMatrix();

	_hash = ff::HashFunc(_transform);
	_hashComputedInverse = _hash;
}

void TransformComponent::Set(ff::Entity entity, const PositionComponent& pos)
{
	ff::hash_t oldHash = _hash;

	DirectX::XMMATRIX matrix = DirectX::XMMatrixAffineTransformation2D(
		DirectX::XMVectorSet(pos._scale, pos._scale, 0, 0),
		DirectX::XMVectorZero(),
		Helpers::DegreesToRadians(pos._rotate),
		DirectX::XMVectorZero());

	DirectX::XMStoreFloat4x4(&_transform, matrix);
	_transform._41 = pos._pos.x;
	_transform._42 = pos._pos.y;

	_hash = ff::HashFunc(_transform);
	_hashComputedInverse = 0;

	if (entity != ff::INVALID_ENTITY && _hash != oldHash)
	{
		entity->TriggerEvent(EntityEvents::ID_TRANSFORM_CHANGED);
	}
}

DirectX::XMMATRIX TransformComponent::GetTransform() const
{
	return DirectX::XMLoadFloat4x4(&GetTransformRaw());
}

DirectX::XMMATRIX TransformComponent::GetInverseTransform() const
{
	return DirectX::XMLoadFloat4x4(&GetInverseTransformRaw());
}

const DirectX::XMFLOAT4X4& TransformComponent::GetTransformRaw() const
{
	return _transform;
}

const DirectX::XMFLOAT4X4& TransformComponent::GetInverseTransformRaw() const
{
	if (_hashComputedInverse != _hash)
	{
		TransformComponent* self = const_cast<TransformComponent*>(this);

		self->_hashComputedInverse = _hash;

		DirectX::XMVECTOR det;
		DirectX::XMStoreFloat4x4(&self->_inverseTransform,
			DirectX::XMMatrixInverse(&det, DirectX::XMLoadFloat4x4(&_transform)));
	}

	return _inverseTransform;
}

ff::PointFixedInt TransformComponent::TransformFromWorld(ff::PointFixedInt pos) const
{
	ff::PointFixedInt outPos;
	TransformFromWorld(&pos, 1, &outPos);
	return outPos;
}

ff::PointFixedInt TransformComponent::TransformToWorld(ff::PointFixedInt pos) const
{
	ff::PointFixedInt outPos;
	TransformToWorld(&pos, 1, &outPos);
	return outPos;
}

static void TransformHelper(DirectX::XMMATRIX matrix, const ff::PointFixedInt* pos, size_t count, ff::PointFixedInt* outPos)
{
	ff::Vector<DirectX::XMFLOAT2, 32> posFloat;
	posFloat.Resize(count);

	for (size_t i = 0; i < count; i++)
	{
		posFloat[i].x = pos[i].x;
		posFloat[i].y = pos[i].y;
	}

	ff::Vector<DirectX::XMFLOAT4, 32> outFloat;
	outFloat.Resize(count);

	DirectX::XMVector2TransformStream(
		outFloat.Data(), sizeof(DirectX::XMFLOAT4),
		posFloat.Data(), sizeof(DirectX::XMFLOAT2), count,
		matrix);

	for (size_t i = 0; i < count; i++)
	{
		outPos[i].SetPoint(outFloat[i].x, outFloat[i].y);
	}
}

void TransformComponent::TransformFromWorld(const ff::PointFixedInt* pos, size_t count, ff::PointFixedInt* outPos) const
{
	::TransformHelper(GetTransform(), pos, count, outPos);
}

void TransformComponent::TransformToWorld(const ff::PointFixedInt* pos, size_t count, ff::PointFixedInt* outPos) const
{
	::TransformHelper(GetInverseTransform(), pos, count, outPos);
}
