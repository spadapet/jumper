#include "pch.h"
#include "Core/CameraView.h"
#include "Core/Helpers.h"

const CameraView& CameraView::Null()
{
	static CameraView view =
	{
		ff::RectFixedInt(0, 0, 1, 1),
		ff::RectFixedInt(0, 0, 1, 1),
		ff::RectFixedInt(0, 0, 1, 1),
		ff::RectFixedInt(0, 0, 1, 1),
		ff::GetIdentityMatrix(),
		ff::GetIdentityMatrix(),
	};

	return view;
}

ff::PointFixedInt CameraView::TransformViewToWorld(ff::PointFixedInt pos) const
{
	DirectX::XMFLOAT2 worldPos;
	DirectX::XMStoreFloat2(&worldPos,
		DirectX::XMVector2Transform(
			Helpers::PointToXMVECTOR(pos),
			DirectX::XMLoadFloat4x4(&_viewToWorldTransform)));

	return ff::PointFixedInt(worldPos.x, worldPos.y);
}

ff::PointFixedInt CameraView::TransformWorldToView(ff::PointFixedInt pos) const
{
	DirectX::XMFLOAT2 viewPos;
	DirectX::XMStoreFloat2(&viewPos,
		DirectX::XMVector2Transform(
			Helpers::PointToXMVECTOR(pos),
			DirectX::XMLoadFloat4x4(&_worldToViewTransform)));

	return ff::PointFixedInt(viewPos.x, viewPos.y);
}
