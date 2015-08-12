#pragma once

#include "Render/RenderWrapper.h"
#include "Systems/SystemBase.h"

class ILevelService;
class Vars;
struct DebugBoundingBoxSystemEntry;
struct DebugHitBoxSystemEntry;
struct DebugPositionSystemEntry;
struct DebugVelocitySystemEntry;

class DebugSystem : public SystemBase, ff::IEntityEventHandler
{
public:
	DebugSystem(ILevelService* levelService);
	virtual ~DebugSystem();

	virtual void Advance(ff::EntityDomain* domain) override;
	virtual void Render(ff::EntityDomain* domain, IRenderQueueService* render) override;

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

private:
	struct CollisionRectInfo
	{
		enum class Type
		{
			Entity,
			Tile,
			TileAngleUp,
			TileAngleDown,
		};

		float _count;
		float _life;
		Type _type;
	};

	void AddCollision(const ff::RectFixedInt& rect, CollisionRectInfo::Type type);
	void RenderBoundingBox(RenderWrapper& render);
	void RenderHitBox(RenderWrapper& render);
	void RenderPosition(RenderWrapper& render);
	void RenderVelocity(RenderWrapper& render);
	void RenderLevelGrid(RenderWrapper& render);
	void RenderCameraFocus(RenderWrapper& render);
	void RenderCollisions(RenderWrapper& render);

	const Vars* _vars;
	ILevelService* _levelService;
	ff::IEntityBucket<DebugBoundingBoxSystemEntry>* _bucketBox;
	ff::IEntityBucket<DebugHitBoxSystemEntry>* _bucketHit;
	ff::IEntityBucket<DebugPositionSystemEntry>* _bucketPos;
	ff::IEntityBucket<DebugVelocitySystemEntry>* _bucketVel;
	ff::Map<ff::RectFixedInt, CollisionRectInfo> _collisions;
	ff::Vector<ff::Entity> _mouseOverEntities;
};
