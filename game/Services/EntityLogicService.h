#pragma once

#include "Core/Globals.h"

enum class TileHitRectType;
enum class TileType;
class ILevelService;
class LogicBase;
struct EntityTileCollision;
struct LogicStateComponent;
struct PositionComponent;
struct VelocityComponent;

class IEntityMoveLogic
{
public:
	virtual void UpdateState(ff::Entity entity, LogicStateComponent& state) = 0;
	virtual void UpdateVelocity(ff::Entity entity, LogicStateComponent& state, VelocityComponent& velocity, const PositionComponent& position) = 0;
	virtual void UpdatePosition(ff::Entity entity, LogicStateComponent& state, const VelocityComponent& velocity, PositionComponent& position) = 0;
};

class IEntityCollisionLogic
{
public:
	virtual void HandleCollision(ff::Entity entity, LogicStateComponent& state, ff::Entity otherEntity) = 0;
};

class IEntityTileCollisionLogic
{
public:
	virtual void HandleTileCollisions(ff::Entity entity, LogicStateComponent& state, const EntityTileCollision* collisions, size_t collisionCount, TileType collisionTypes, TileHitRectType hitRectTypes) = 0;
};

class IEntityLogicService
{
public:
	virtual IEntityMoveLogic* GetMoveLogic(Advancing::Type type) = 0;
	virtual IEntityCollisionLogic* GetCollisionLogic(Advancing::Type type) = 0;
	virtual IEntityTileCollisionLogic* GetTileCollisionLogic(Advancing::Type type) = 0;
};

class EntityLogicService : public IEntityLogicService
{
public:
	EntityLogicService(ILevelService* levelService);
	~EntityLogicService();

	virtual IEntityMoveLogic* GetMoveLogic(Advancing::Type type) override;
	virtual IEntityCollisionLogic* GetCollisionLogic(Advancing::Type type) override;
	virtual IEntityTileCollisionLogic* GetTileCollisionLogic(Advancing::Type type) override;

private:
	LogicBase* GetLogicBase(Advancing::Type type);

	ILevelService* _levelService;
	ff::Map<Advancing::Type, std::shared_ptr<LogicBase>> _logics;
};
