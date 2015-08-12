#include "pch.h"
#include "Components/TilesComponent.h"
#include "Core/Globals.h"
#include "Core/Helpers.h"
#include "Core/TileSet.h"
#include "Services/CollisionService.h"

bool TileCollision::operator<(const TileCollision& rhs) const
{
	if (_worldRect.left == rhs._worldRect.left)
	{
		return _worldRect.top < rhs._worldRect.top;
	}

	return _worldRect.left < rhs._worldRect.left;
}

ff::PointFixedInt TileCollision::GetTileOrigin() const
{
	return _layer->GetComponent<TilesComponent>()->_tileSize * _tileIndex.ToType<ff::FixedInt>();
}

bool EntityTileCollision::operator<(const EntityTileCollision& rhs) const
{
	if (_entity == rhs._entity)
	{
		return TileCollision::operator<(rhs);
	}

	return _entity->GetId() < rhs._entity->GetId();
}

EntityCollisionStats::EntityCollisionStats()
{
	ff::ZeroObject(*this);
}

TileCollisionStats::TileCollisionStats()
{
	ff::ZeroObject(*this);
}
