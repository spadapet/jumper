#pragma once

#include "Services/CollisionService.h"
#include "Systems/SystemBase.h"

class ILevelService;
struct TileCollisionSystemEntry;

class TileCollisionSystem : public SystemBase
{
public:
	TileCollisionSystem(ILevelService* levelService);

	const TileCollisionStats& GetStats() const;

	// SystemBase
	virtual void Advance(ff::EntityDomain* domain) override;

private:
	void BuildCollisionEntries();

	struct CollisionEntry
	{
		TileCollisionSystemEntry* _entry;
		size_t _collisionStart;
		size_t _collisionCount;
		TileType _collisionTileTypes;
		TileHitRectType _collisionHitRectTypes;
	};

	ILevelService* _levelService;
	ff::IEntityBucket<TileCollisionSystemEntry>* _bucket;
	ff::Vector<EntityTileCollision> _collisions;
	ff::Vector<CollisionEntry> _collisionEntries;
	TileCollisionStats _stats;
};
