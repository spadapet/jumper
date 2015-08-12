#pragma once

#include "Services/CollisionService.h"
#include "Systems/SystemBase.h"

struct CollisionSystemEntry;
struct CollisionLayerEntry;
struct Tile;

class CollisionSystem
	: public SystemBase
	, public ICollisionService
	, public ff::IEntityEventHandler
	, public ff::IEntityBucketListener<CollisionSystemEntry>
{
public:
	CollisionSystem(ff::EntityDomain* domain, ff::PointFixedInt cellSize);
	virtual ~CollisionSystem();

	// ICollisionService
	virtual bool GetEntityCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityCollision>& collisions, EntityCollisionStats& stats) override;
	virtual bool GetTileCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityTileCollision>& tileCollisions, TileCollisionStats& stats) override;
	virtual bool EntityHitTest(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results, bool testRotatedRectangle) override;
	virtual bool TileHitTest(const ff::RectFixedInt& rect, TileType& resultTileTypes, TileHitRectType& resultHitRectTypes, ff::ItemCollector<TileCollision>& results) override;

	// IEntityEventHandler
	virtual void OnEntityDeleted(ff::Entity entity) override;
	virtual void OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs) override;

	// IEntityBucketListener
	virtual void OnEntryAdded(CollisionSystemEntry& entry) override;
	virtual void OnEntryRemoving(CollisionSystemEntry& entry) override;

private:
	void UpdateEntity(ff::Entity entity);
	void UpdateEntity(CollisionSystemEntry& entry);
	ff::RectInt RectToCells(const ff::RectFixedInt& rect) const;
	void ClearCells(CollisionSystemEntry& entry);
	bool HitTest(const CollisionSystemEntry& entry1, const CollisionSystemEntry& entry2);
	bool GetLayerEntries(const ff::RectFixedInt& rect, ff::ItemCollector<CollisionLayerEntry*>& layerEntries);
	bool TileHitTest(const ff::RectFixedInt& rect, const ff::Vector<CollisionLayerEntry*, 32>& layerEntries, TileType& resultTileTypes, TileHitRectType& resultHitRectTypes, ff::ItemCollector<TileCollision>& results);

	ff::EntityDomain* _domain;
	ff::IEntityBucket<CollisionSystemEntry>* _bucketGrid;
	ff::IEntityBucket<CollisionLayerEntry>* _bucketLayer;
	ff::Map<ff::PointInt, CollisionSystemEntry*> _cells;
	ff::Set<ff::hash_t, ff::NonHasher<ff::hash_t>> _testedCollisions;
	ff::PointFixedInt _cellSize;
	size_t _hitTestCount;
	size_t _tileHitTestCount;
};
