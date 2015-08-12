#pragma once

#include "Logic/LogicBase.h"

class DebugCameraLogic : public LogicBase
{
public:
	DebugCameraLogic();

	virtual void UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position) override;
};
