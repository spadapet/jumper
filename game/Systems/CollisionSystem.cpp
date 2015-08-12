#include "pch.h"
#include "Components/EntityTypeComponent.h"
#include "Components/HitBoxComponent.h"
#include "Components/PositionComponent.h"
#include "Components/PositionToHitBoxComponent.h"
#include "Components/TilesComponent.h"
#include "Components/TransformComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Systems/CollisionSystem.h"
#include "Types/Timer.h"

static bool HitTestHelper(const ff::PointFixedInt* aabb, const ff::PointFixedInt* points)
{
	DirectX::XMVECTOR otherX = DirectX::XMVectorSet(points[0].x, points[1].x, points[2].x, points[3].x);
	DirectX::XMVECTOR otherY = DirectX::XMVectorSet(points[0].y, points[1].y, points[2].y, points[3].y);

	if (DirectX::XMVector4LessOrEqual(otherX, DirectX::XMVectorReplicate(aabb[0].x)) ||
		DirectX::XMVector4GreaterOrEqual(otherX, DirectX::XMVectorReplicate(aabb[2].x)) ||
		DirectX::XMVector4LessOrEqual(otherY, DirectX::XMVectorReplicate(aabb[0].y)) ||
		DirectX::XMVector4GreaterOrEqual(otherY, DirectX::XMVectorReplicate(aabb[2].y)))
	{
		return false;
	}

	return true;
}

struct CollisionSystemEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS2(CollisionSystemEntry)

	bool Filter(EntityType typeFilter) const;

	HitBoxComponent* _box;
	PositionToHitBoxComponent* _posToBox;
	TransformComponent* _transform;
	EntityTypeComponent* _type;

	ff::hash_t _boxHash;
	ff::RectInt _cells;
	size_t _hitTestCount;
	size_t _tileHitTestCount;
};

BEGIN_ENTRY_COMPONENTS(CollisionSystemEntry)
HAS_COMPONENT(HitBoxComponent)
HAS_COMPONENT(PositionToHitBoxComponent)
HAS_COMPONENT(TransformComponent)
HAS_OPTIONAL_COMPONENT(EntityTypeComponent)
END_ENTRY_COMPONENTS(CollisionSystemEntry)

CollisionSystemEntry::CollisionSystemEntry()
	: _boxHash(0)
	, _cells(0, 0, -1, -1)
	, _hitTestCount(0)
	, _tileHitTestCount(0)
{
}

bool CollisionSystemEntry::Filter(EntityType typeFilter) const
{
	if (!_type)
	{
		return typeFilter == EntityType::Any;
	}

	return _type->HasAnyType(typeFilter);
}

struct CollisionLayerEntry : public ff::EntityBucketEntry
{
	DECLARE_ENTRY_COMPONENTS()

	TilesComponent* _tiles;
	PositionComponent* _pos;
};

BEGIN_ENTRY_COMPONENTS(CollisionLayerEntry)
HAS_COMPONENT(TilesComponent)
HAS_COMPONENT(PositionComponent)
END_ENTRY_COMPONENTS(CollisionLayerEntry)

CollisionSystem::CollisionSystem(ff::EntityDomain* domain, ff::PointFixedInt cellSize)
	: _domain(domain)
	, _bucketGrid(domain->GetBucket<CollisionSystemEntry>())
	, _bucketLayer(domain->GetBucket<CollisionLayerEntry>())
	, _cellSize(cellSize)
	, _hitTestCount(0)
	, _tileHitTestCount(0)
{
	_domain->AddEventHandler(EntityEvents::ID_HIT_BOX_CHANGED, this);
	_bucketGrid->AddListener(this);

	_cells.SetBucketCount(Constants::BIG_ENTITY_BUCKET_SIZE);
	_testedCollisions.SetBucketCount(Constants::REASONABLE_ENTITY_BUCKET_SIZE);
}

CollisionSystem::~CollisionSystem()
{
	_domain->RemoveEventHandler(EntityEvents::ID_HIT_BOX_CHANGED, this);
	_bucketGrid->RemoveListener(this);
}

bool CollisionSystem::GetEntityCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityCollision>& collisions, EntityCollisionStats& stats)
{
	ff::RectInt cells = RectToCells(rect);
	bool result = false;

	stats = EntityCollisionStats();
	stats._ticks = ff::Timer::GetCurrentRawTime();
	stats._cellCheckCount = (size_t)cells.Area();

	for (ff::PointInt xy = cells.TopLeft(); xy.x <= cells.right; xy.x++)
	{
		for (xy.y = cells.top; xy.y <= cells.bottom; xy.y++)
		{
			for (auto i = _cells.GetKey(xy); i; i = _cells.GetNextDupeKey(*i))
			{
				CollisionSystemEntry& entry1 = *i->GetEditableValue();
				if (!entry1.Filter(typeFilter))
				{
					continue;
				}

				for (auto h = _cells.GetNextDupeKey(*i); h; h = _cells.GetNextDupeKey(*h))
				{
					CollisionSystemEntry& entry2 = *h->GetEditableValue();
					if (!entry2.Filter(typeFilter))
					{
						continue;
					}

					EntityCollision ec;
					ec._entity1 = std::min(entry1._entity, entry2._entity);
					ec._entity2 = std::max(entry1._entity, entry2._entity);
					stats._entitySameCellCount++;

					ff::hash_t ecHash = ff::HashFunc(ec);
					if (!_testedCollisions.KeyExists(ecHash))
					{
						_testedCollisions.SetKey(ecHash);
						stats._entityCheckCount++;

						if (HitTest(entry1, entry2))
						{
							collisions.Push(ec);
							stats._entityHitCount++;
							result = true;
						}
					}
				}
			}
		}
	}

	_testedCollisions.Clear();
	stats._ticks = ff::Timer::GetCurrentRawTime() - stats._ticks;

	return result;
}

bool CollisionSystem::GetTileCollisions(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<EntityTileCollision>& tileCollisions, TileCollisionStats& stats)
{
	stats = TileCollisionStats();

	ff::Vector<CollisionLayerEntry*, 32> layerEntries;
	noAssertRetVal(GetLayerEntries(rect, layerEntries.GetCollector()), false);

	ff::RectInt cells = RectToCells(rect);
	bool result = false;

	_tileHitTestCount++;
	stats._ticks = ff::Timer::GetCurrentRawTime();
	stats._cellCheckCount = (size_t)cells.Area();
	stats._layerCount = layerEntries.Size();

	for (ff::PointInt xy = cells.TopLeft(); xy.x <= cells.right; xy.x++)
	{
		for (xy.y = cells.top; xy.y <= cells.bottom; xy.y++)
		{
			for (auto i = _cells.GetKey(xy); i; i = _cells.GetNextDupeKey(*i))
			{
				CollisionSystemEntry& entry = *i->GetEditableValue();

				if (entry._tileHitTestCount != _tileHitTestCount)
				{
					entry._tileHitTestCount = _tileHitTestCount;

					if (entry.Filter(EntityType::CollidesWithTiles) && entry.Filter(typeFilter))
					{
						ff::Vector<TileCollision, 32> entityTileCollisions;
						TileType resultTypes = TileType::None;
						TileHitRectType resultHitRectTypes = TileHitRectType::None;
						stats._entityCheckCount++;

						if (TileHitTest(entry._box->_box, layerEntries, resultTypes, resultHitRectTypes, entityTileCollisions.GetCollector()))
						{
							stats._entityHitCount++;

							for (const TileCollision& tc : entityTileCollisions)
							{
								EntityTileCollision etc;
								static_cast<TileCollision&>(etc) = tc;
								etc._entity = entry._entity;

								tileCollisions.Push(etc);
								stats._tileHitCount++;
								result = true;
							}
						}
					}
				}
			}
		}
	}

	stats._ticks = ff::Timer::GetCurrentRawTime() - stats._ticks;

	return result;
}

bool CollisionSystem::EntityHitTest(const ff::RectFixedInt& rect, EntityType typeFilter, ff::ItemCollector<ff::Entity>& results, bool testRotatedRectangle)
{
	ff::RectInt cells = RectToCells(rect);
	bool result = false;

	_hitTestCount++;

	for (ff::PointInt xy = cells.TopLeft(); xy.x <= cells.right; xy.x++)
	{
		for (xy.y = cells.top; xy.y <= cells.bottom; xy.y++)
		{
			for (auto i = _cells.GetKey(xy); i; i = _cells.GetNextDupeKey(*i))
			{
				CollisionSystemEntry& entry = *i->GetEditableValue();
				if (entry._hitTestCount == _hitTestCount || !entry.Filter(typeFilter))
				{
					continue;
				}

				entry._hitTestCount = _hitTestCount;

				if (entry._box->_box.DoesTouch(rect))
				{
					if (!testRotatedRectangle ||
						entry._box->_points[0].x == entry._box->_points[1].x ||
						entry._box->_points[0].y == entry._box->_points[1].y)
					{
						// AABB is good enough when there is no rotation
						results.Push(entry._entity);
						result = true;
					}
					else
					{
						ff::PointFixedInt points[4] =
						{
							rect.TopLeft(),
							rect.TopRight(),
							rect.BottomRight(),
							rect.BottomLeft(),
						};

						// transform the other rectangle into my own coordinate system
						ff::PointFixedInt transformedPoints[4];
						entry._transform->TransformFromWorld(points, 4, transformedPoints);

						if (::HitTestHelper(entry._posToBox->_points, transformedPoints))
						{
							results.Push(entry._entity);
							result = true;
						}
					}
				}
			}
		}
	}

	return result;
}

bool CollisionSystem::TileHitTest(const ff::RectFixedInt& rect, TileType& resultTileTypes, TileHitRectType& resultHitRectTypes, ff::ItemCollector<TileCollision>& results)
{
	resultTileTypes = TileType::None;

	ff::Vector<CollisionLayerEntry*, 32> layerEntries;
	noAssertRetVal(GetLayerEntries(rect, layerEntries.GetCollector()), false);

	return TileHitTest(rect, layerEntries, resultTileTypes, resultHitRectTypes, results);
}

bool CollisionSystem::GetLayerEntries(const ff::RectFixedInt& rect, ff::ItemCollector<CollisionLayerEntry*>& layerEntries)
{
	ff::Vector<ff::Entity, 32> layerEntities;
	noAssertRetVal(EntityHitTest(rect, EntityType::Layer, layerEntities.GetCollector(), false), false);

	bool added = false;

	for (ff::Entity entity : layerEntities)
	{
		CollisionLayerEntry* entry = _bucketLayer->GetEntry(entity);
		if (entry)
		{
			layerEntries.Push(entry);
			added = true;
		}
	}

	return added;
}

bool CollisionSystem::TileHitTest(
	const ff::RectFixedInt& rect,
	const ff::Vector<CollisionLayerEntry*, 32>& layerEntries,
	TileType& resultTileTypes,
	TileHitRectType& resultHitRectTypes,
	ff::ItemCollector<TileCollision>& results)
{
	resultTileTypes = TileType::None;
	resultHitRectTypes = TileHitRectType::None;

	bool added = false;

	for (CollisionLayerEntry* entry : layerEntries)
	{
		ff::PointFixedInt tileSize = entry->_tiles->_tileSize;
		ff::PointFixedInt layerPos = entry->_pos->_pos;
		ff::RectFixedInt layerRect = rect - layerPos;

		ff::RectInt cells(
			Helpers::PointToCell(layerRect.TopLeft(), tileSize) - ff::PointInt::Ones(),
			Helpers::PointToCell(layerRect.BottomRight(), tileSize));
		cells.left = std::max(cells.left, 0);
		cells.top = std::max(cells.top, 0);
		cells.right = std::min(cells.right, entry->_tiles->_size.x - 1);
		cells.bottom = std::min(cells.bottom, entry->_tiles->_size.y - 1);

		for (int y = cells.top; y <= cells.bottom; y++)
		{
			const TilesComponent::TileInfo* tileInfo = &entry->_tiles->_tiles.GetAt(y * entry->_tiles->_size.x + cells.left);
			ff::RectFixedInt tileRect = ff::RectFixedInt(tileSize).Offset(tileSize.x * cells.left + layerPos.x, tileSize.y * y + layerPos.y);

			for (int x = cells.left; x <= cells.right; x++, tileInfo++, tileRect.left += tileSize.x, tileRect.right += tileSize.x)
			{
				if (tileInfo->_tile && tileInfo->_tile->_hitRects.Size() && rect.DoesTouch(tileRect))
				{
					ff::RectFixedInt tileIntersect = rect.Intersect(tileRect).Offset(-tileRect.TopLeft());

					for (const TileHitRect& subHitRect : tileInfo->_tile->_hitRects)
					{
						if (tileIntersect.DoesTouch(subHitRect._rect))
						{
							TileCollision tc;
							tc._layer = entry->_entity;
							tc._tileIndex = ff::PointInt(x, y);
							tc._tileRect = tileIntersect.Intersect(subHitRect._rect);
							tc._worldRect = tc._tileRect.Offset(tileRect.TopLeft());
							tc._tile = tileInfo->_tile;
							tc._tileHitRect = &subHitRect;

							resultTileTypes = Helpers::SetFlags(resultTileTypes, tileInfo->_tile->_type);
							resultHitRectTypes = Helpers::SetFlags(resultHitRectTypes, subHitRect._type);
							results.Push(std::move(tc));
							added = true;
						}
					}
				}
			}
		}
	}

	return added;
}

void CollisionSystem::OnEntityDeleted(ff::Entity entity)
{
}

void CollisionSystem::OnEntityEvent(ff::Entity entity, ff::hash_t eventId, ff::EntityEventArgs* eventArgs)
{
	UpdateEntity(entity);
}

void CollisionSystem::OnEntryAdded(CollisionSystemEntry& entry)
{
	UpdateEntity(entry);
}

void CollisionSystem::OnEntryRemoving(CollisionSystemEntry& entry)
{
	ClearCells(entry);
}

ff::RectInt CollisionSystem::RectToCells(const ff::RectFixedInt& rect) const
{
	return ff::RectInt(
		Helpers::PointToCell(rect.TopLeft(), _cellSize),
		Helpers::PointToCell(rect.BottomRight(), _cellSize));
}

void CollisionSystem::ClearCells(CollisionSystemEntry& entry)
{
	for (ff::PointInt xy = entry._cells.TopLeft(); xy.y <= entry._cells.bottom; xy.y++)
	{
		for (xy.x = entry._cells.left; xy.x <= entry._cells.right; xy.x++)
		{
			for (auto i = _cells.GetKey(xy); i; i = _cells.GetNextDupeKey(*i))
			{
				if (i->GetValue() == &entry)
				{
					_cells.DeleteKey(*i);
					break;
				}
			}
		}
	}
}

void CollisionSystem::UpdateEntity(ff::Entity entity)
{
	CollisionSystemEntry* entry = _bucketGrid->GetEntry(entity);
	noAssertRet(entry);
	UpdateEntity(*entry);
}

void CollisionSystem::UpdateEntity(CollisionSystemEntry& entry)
{
	if (entry._boxHash != entry._box->_hash)
	{
		entry._boxHash = entry._box->_hash;
		ff::RectInt cells = RectToCells(entry._box->_box);

		if (entry._cells != cells)
		{
			ClearCells(entry);

			entry._hitTestCount = 0;
			entry._tileHitTestCount = 0;
			entry._cells = cells;

			for (ff::PointInt xy = cells.TopLeft(); xy.y <= cells.bottom; xy.y++)
			{
				for (xy.x = cells.left; xy.x <= cells.right; xy.x++)
				{
					_cells.InsertKey(xy, &entry);
				}
			}
		}
	}
}

bool CollisionSystem::HitTest(const CollisionSystemEntry& entry1, const CollisionSystemEntry& entry2)
{
	const ff::PointFixedInt* points1 = entry1._box->_points;
	const ff::PointFixedInt* points2 = entry2._box->_points;

	if ((points1[0].x == points1[1].x || points1[0].y == points1[1].y) &&
		(points2[0].x == points2[1].x || points2[0].y == points2[1].y))
	{
		// AABB is good enough when there is no rotation

		return entry1._box->_box.DoesTouch(entry2._box->_box);
	}
	else
	{
		// test two rotated rectangles
		ff::PointFixedInt testPoints[4];
		entry1._transform->TransformFromWorld(points2, 4, testPoints);

		if (::HitTestHelper(entry1._posToBox->_points, testPoints))
		{
			return true;
		}

		// transform my rectangle into the other coordinate system
		entry2._transform->TransformFromWorld(points1, 4, testPoints);

		if (::HitTestHelper(entry2._posToBox->_points, testPoints))
		{
			return true;
		}
	}

	return false;
}
