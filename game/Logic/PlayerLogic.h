#pragma once

#include "Logic/LogicBase.h"

class ILevelService;
struct PlayerLogicEntry;

class PlayerLogic : public LogicBase
{
public:
	PlayerLogic(ILevelService* levelService);

	virtual void UpdateState(ff::Entity entity, LogicStateComponent& state) override;
	virtual void UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position) override;
	virtual void UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position) override;
	virtual void HandleTileCollisions(ff::Entity entity, LogicStateComponent& state, const EntityTileCollision* collisions, size_t collisionCount, TileType collisionTypes, TileHitRectType hitRectTypes) override;

private:
	ILevelService* _levelService;
	ff::IEntityBucket<PlayerLogicEntry>* _bucket;
};
