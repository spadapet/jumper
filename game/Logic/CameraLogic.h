#pragma once

#include "Logic/LogicBase.h"

class ICameraService;
class Vars;

class CameraLogic : public LogicBase
{
public:
	CameraLogic(ICameraService* cameraService, const Vars* vars);

	virtual void UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position) override;
	virtual void UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position) override;

private:
	ICameraService* _cameraService;
	const Vars* _vars;
};
