#pragma once

#include "Core/CameraView.h"

enum class DebugCameraType
{
	None,
	Zoom2,
	Zoom4,

	Count
};

struct CameraComponent : public ff::Component
{
	CameraComponent();

	void SetFollow(ff::Entity entity);
	ff::Entity GetFollow() const;

	ff::WeakEntity _followEntity;
	DebugCameraType _debugType;
	CameraView _view;
};
