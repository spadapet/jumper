#include "pch.h"
#include "Components/CameraComponent.h"
#include "Components/PositionComponent.h"
#include "Logic/DebugCameraLogic.h"

DebugCameraLogic::DebugCameraLogic()
{
}

void DebugCameraLogic::UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position)
{
	const CameraComponent& cameraComponent = *entity->GetComponent<CameraComponent>();
	ff::Entity followEntity = cameraComponent.GetFollow();
	noAssertRet(followEntity != ff::INVALID_ENTITY);

	const PositionComponent& followPos = *followEntity->GetComponent<PositionComponent>();
	position.Set(entity, followPos._pos);
}
