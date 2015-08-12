#include "pch.h"
#include "Core/CameraView.h"
#include "Core/Vars.h"
#include "Components/CameraComponent.h"
#include "Components/LogicStateComponent.h"
#include "Components/PositionComponent.h"
#include "Components/VelocityComponent.h"
#include "Logic/CameraLogic.h"
#include "Logic/LogicHelpers.h"
#include "Services/CameraService.h"

enum class FollowState
{
	Unknown,
	FocusLeft,
	FocusRight,
};

struct CameraLogicState : public ff::Component
{
	FollowState _followState;
	ff::PointFixedInt _currentPos;
	ff::PointFixedInt _destPos;
};

CameraLogic::CameraLogic(ICameraService* cameraService, const Vars* vars)
	: _cameraService(cameraService)
	, _vars(vars)
{
}

void CameraLogic::UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position)
{
	const CameraComponent& cameraComponent = *entity->GetComponent<CameraComponent>();
	const CameraView& cameraView = cameraComponent._view;
	ff::Entity followEntity = cameraComponent.GetFollow();
	noAssertRet(followEntity != ff::INVALID_ENTITY);

	const PositionComponent* followEntityPos = followEntity->GetComponent<PositionComponent>();
	const VelocityComponent* followEntityVel = followEntity->GetComponent<VelocityComponent>();
	assertRet(followEntityPos);

	ff::PointFixedInt followPos = followEntityPos->_pos;
	if (followEntityVel)
	{
		followPos += followEntityVel->_posVelocity;
	}

	CameraLogicState* cameraLogic = entity->GetComponent<CameraLogicState>();
	if (!cameraLogic)
	{
		cameraLogic = entity->AddComponent<CameraLogicState>();
		cameraLogic->_followState = FollowState::Unknown;
		cameraLogic->_currentPos = position._pos;
		cameraLogic->_destPos = position._pos;
	}

	const ff::RectFixedInt& viewBounds = cameraView._worldBounds;
	const ff::FixedInt focusWidth = _vars->GetCameraViewFocusWidth() ? viewBounds.Width() / _vars->GetCameraViewFocusWidth() : 0_f;
	const ff::FixedInt focusOffsetFromCenter = _vars->GetCameraViewFocusOffsetX() ? viewBounds.Width() / _vars->GetCameraViewFocusOffsetX() : 0_f;

	const ff::FixedInt focusLeft[2] =
	{
		cameraLogic->_destPos.x - focusOffsetFromCenter - focusWidth,
		cameraLogic->_destPos.x - focusOffsetFromCenter,
	};

	const ff::FixedInt focusRight[2] =
	{
		cameraLogic->_destPos.x + focusOffsetFromCenter,
		cameraLogic->_destPos.x + focusOffsetFromCenter + focusWidth,
	};

	if (followPos.x < focusLeft[0])
	{
		cameraLogic->_followState = FollowState::FocusRight;
	}
	else if (followPos.x >= focusRight[1])
	{
		cameraLogic->_followState = FollowState::FocusLeft;
	}

	switch (cameraLogic->_followState)
	{
	case FollowState::Unknown:
	case FollowState::FocusLeft:
		if (followPos.x >= focusLeft[1])
		{
			cameraLogic->_destPos.x = followPos.x + focusOffsetFromCenter;
		}
		break;

	case FollowState::FocusRight:
		if (followPos.x < focusRight[0])
		{
			cameraLogic->_destPos.x = followPos.x - focusOffsetFromCenter;
		}
		break;
	}

	ff::RectFixedInt cameraWorldBounds = _cameraService->GetCameraBounds().Deflate(cameraView._worldBounds.Size() / 2);
	ff::PointFixedInt destVelocity = LogicHelpers::Clamp(cameraLogic->_destPos, cameraWorldBounds) - cameraLogic->_currentPos;

	if (std::abs(destVelocity.x) > _vars->GetCameraVelocityThreshold())
	{
		destVelocity.x = _vars->GetCameraVelocityThresholdDivisor()
			? destVelocity.x / _vars->GetCameraVelocityThresholdDivisor() + std::copysign(_vars->GetCameraVelocityThreshold(), destVelocity.x)
			: 0_f;
	}

	if (std::abs(destVelocity.y) > _vars->GetCameraVelocityThreshold())
	{
		destVelocity.y = _vars->GetCameraVelocityThresholdDivisor()
			? destVelocity.y / _vars->GetCameraVelocityThresholdDivisor() + std::copysign(_vars->GetCameraVelocityThreshold(), destVelocity.y)
			: 0_f;
	}

	destVelocity = LogicHelpers::Clamp(destVelocity, _vars->GetCameraVelocityLimits());
	velocity._posVelocity = LogicHelpers::AdjustTowardsValue(velocity._posVelocity, destVelocity, _vars->GetCameraAcceleration());
	velocity._posVelocity = LogicHelpers::Clamp(velocity._posVelocity, cameraWorldBounds - cameraLogic->_currentPos);
}

void CameraLogic::UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position)
{
	CameraLogicState* cameraLogic = entity->GetComponent<CameraLogicState>();
	if (cameraLogic)
	{
		// Round the camera position (and therefore boundary) to pixel positions
		cameraLogic->_currentPos += velocity._posVelocity;
		ff::PointFixedInt roundedPos(std::floor(cameraLogic->_currentPos.x), std::floor(cameraLogic->_currentPos.y));
		position.Set(entity, roundedPos);
	}
	else
	{
		LogicBase::UpdatePosition(entity, state, velocity, position);
	}
}
