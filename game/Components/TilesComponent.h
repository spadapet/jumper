#pragma once

struct Tile;
struct LevelData;

struct TilesComponent : public ff::Component
{
	TilesComponent();

	struct TileInfo
	{
		// Must only contain POD since memcpy is used during resize
		const Tile* _tile;
	};

	void Init(const LevelData& levelData, size_t layer);
	void Resize(ff::PointInt size);
	const TileInfo* GetTile(ff::PointInt index) const;

	ff::PointInt _size;
	ff::PointFixedInt _tileSize;
	ff::Vector<TileInfo> _tiles;
	XMCOLOR _color;
};
