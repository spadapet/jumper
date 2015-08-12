#pragma once

#include "./TileSet.h"
#include "Resource/ResourcePersist.h"
#include "Resource/ResourceValue.h"

struct LevelDataInternal;

struct LevelData
{
	struct Layer
	{
		ff::String _name;
		ff::PointInt _size;
		ff::PointFixedInt _position;
		ff::Vector<const Tile*> _tiles;
		ff::FixedInt _opacity;
		bool _visible;
	};

	struct TileSet
	{
		::TileSet _tileSet;
		size_t _firstId;
		size_t _countId;
	};

	ff::String _name;
	ff::PointInt _size;
	ff::PointFixedInt _tileSize;
	ff::PointFixedInt _playerStart;
	ff::Vector<Layer> _layers;
	ff::Vector<TileSet> _tileSets;
};

class __declspec(uuid("6ffe31fb-b366-46ea-b413-b522ef06f402"))
	LevelDataResource
	: public ff::ComBase
	, public ff::IResourcePersist
{
public:
	DECLARE_HEADER(LevelDataResource);

	bool IsLevelDataFullyLoaded() const;
	LevelData GetLevelData() const;

	// IResourceSave
	virtual bool LoadFromSource(const ff::Dict& dict) override;
	virtual bool LoadFromCache(const ff::Dict& dict) override;
	virtual bool SaveToCache(ff::Dict& dict) override;

private:
	bool LoadTileLayerFromSource(const ff::Dict& dict);
	bool LoadObjectLayerFromSource(const ff::Dict& dict);
	bool LoadTileSetFromSource(ff::IResourceLoadListener* loadListener, const ff::Dict& dict);
	bool LoadTileLayerFromCache(const ff::Dict& dict);
	bool LoadTileSetFromCache(const ff::Dict& dict);

	std::unique_ptr<LevelDataInternal> _levelData;
};
