#include "pch.h"
#include "Core/Vars.h"
#include "Components/CameraComponent.h"

CameraComponent::CameraComponent()
	: _debugType(DebugCameraType::None)
{
}

void CameraComponent::SetFollow(ff::Entity entity)
{
	_followEntity = entity;
}

ff::Entity CameraComponent::GetFollow() const
{
	return _followEntity;
}
