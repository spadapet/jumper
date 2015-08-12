#include "pch.h"
#include "Components/LogicStateComponent.h"
#include "Components/PositionComponent.h"
#include "Logic/LogicBase.h"

LogicBase::~LogicBase()
{
}

void LogicBase::UpdateState(ff::Entity entity, LogicStateComponent& state)
{
}

void LogicBase::UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position)
{
}

void LogicBase::UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position)
{
	position.Update(entity, velocity);
}

void LogicBase::HandleCollision(ff::Entity entity, LogicStateComponent& state, ff::Entity otherEntity)
{
}

void LogicBase::HandleTileCollisions(ff::Entity entity, LogicStateComponent& state, const EntityTileCollision* collisions, size_t collisionCount, TileType collisionTypes, TileHitRectType)
{
}
