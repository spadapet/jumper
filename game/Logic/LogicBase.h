#pragma once

#include "Services/EntityLogicService.h"

struct EntityTileCollision;

class LogicBase
	: public IEntityMoveLogic
	, public IEntityCollisionLogic
	, public IEntityTileCollisionLogic
{
public:
	virtual ~LogicBase();

	// IEntityMoveLogic
	virtual void UpdateState(ff::Entity entity, LogicStateComponent& state) override;
	virtual void UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position) override;
	virtual void UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position) override;

	// IEntityCollisionLogic
	virtual void HandleCollision(ff::Entity entity, LogicStateComponent& state, ff::Entity otherEntity) override;

	// IEntityTileCollisionLogic
	virtual void HandleTileCollisions(ff::Entity entity, LogicStateComponent& state, const EntityTileCollision* collisions, size_t collisionCount, TileType collisionTypes, TileHitRectType hitRectTypes) override;
};
