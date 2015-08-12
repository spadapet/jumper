#include "pch.h"
#include "Core/LevelData.h"
#include "Core/TileSet.h"
#include "Components/TilesComponent.h"
#include "Data/Data.h"
#include "Resource/ResourceValue.h"

TilesComponent::TilesComponent()
	: _size(ff::PointInt::Zeros())
	, _tileSize(ff::PointFixedInt::Zeros())
	, _color(ff::GetColorWhite())
{
}

void TilesComponent::Init(const LevelData& levelData, size_t layer)
{
	const LevelData::Layer& layerData = levelData._layers[layer];
	_size = layerData._size;
	_tileSize = levelData._tileSize;
	_color = XMCOLOR(1, 1, 1, layerData._opacity);

	_tiles.Resize(layerData._tiles.Size());
	for (size_t i = 0; i < _tiles.Size(); i++)
	{
		TileInfo& tile = _tiles[i];
		tile._tile = layerData._tiles[i];
	}
}

void TilesComponent::Resize(ff::PointInt size)
{
	ff::Vector<TileInfo> tiles;
	tiles.Resize(size.x * size.y);
	std::memset(tiles.Data(), 0, tiles.ByteSize());

	for (int y = 0, copyX = std::min(size.x, _size.x); y < std::min(size.y, _size.y); y++)
	{
		std::memcpy(&tiles[y * size.x], &_tiles[y * _size.x], sizeof(TileInfo) * copyX);
	}

	_tiles = std::move(tiles);
	_size = size;
}

const TilesComponent::TileInfo* TilesComponent::GetTile(ff::PointInt index) const
{
	if (index.x >= 0 && index.x < _size.x && index.y >= 0 && index.y < _size.y)
	{
		return &_tiles[index.y * _size.x + index.x];
	}

	return nullptr;
}
