#pragma once

enum class EntityType;
enum class TileHitRectType;
enum class TileType;
struct Tile;
struct TileHitRect;

struct TileCollision
{
	bool operator<(const TileCollision& rhs) const;
	ff::PointFixedInt GetTileOrigin() const;

	ff::Entity _layer;
	ff::PointInt _tileIndex;
	ff::RectFixedInt _tileRect;
	ff::RectFixedInt _worldRect;
	const Tile* _tile;
	const TileHitRect* _tileHitRect;
};

struct EntityTileCollision : public TileCollision
{
	bool operator<(const EntityTileCollision& rhs) const;

	ff::Entity _entity;
};

struct EntityCollision
{
	ff::Entity _entity1;
	ff::Entity _entity2;
};

struct EntityCollisionStats
{
	EntityCollisionStats();

	UINT64 _ticks;
	size_t _cellCheckCount;
	size_t _entitySameCellCount;
	size_t _entityCheckCount;
	size_t _entityHitCount;
};

struct TileCollisionStats
{
	TileCollisionStats();

	UINT64 _ticks;
	size_t _cellCheckCount;
	size_t _layerCount;
	size_t _entityCheckCount;
	size_t _entityHitCount;
	size_t _tileHitCount;
};

class ICollisionService
{
public:
	virtual bool GetEntityCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityCollision>& collisions, EntityCollisionStats& stats) = 0;
	virtual bool GetTileCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityTileCollision>& tileCollisions, TileCollisionStats& stats) = 0;
	virtual bool EntityHitTest(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results, bool testRotatedRectangle) = 0;
	virtual bool TileHitTest(const ff::RectFixedInt& rect, TileType& resultTileTypes, TileHitRectType& resultHitRectTypes, ff::ItemCollector<TileCollision>& results) = 0;
};
