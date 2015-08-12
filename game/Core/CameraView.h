#pragma once

struct CameraView
{
	static const CameraView& Null();

	ff::PointFixedInt TransformViewToWorld(ff::PointFixedInt pos) const;
	ff::PointFixedInt TransformWorldToView(ff::PointFixedInt pos) const;

	ff::RectFixedInt _view;
	ff::RectFixedInt _worldBounds;
	ff::RectFixedInt _worldRenderBounds;
	ff::RectFixedInt _worldUpdateBounds;
	DirectX::XMFLOAT4X4 _viewToWorldTransform;
	DirectX::XMFLOAT4X4 _worldToViewTransform;
};
