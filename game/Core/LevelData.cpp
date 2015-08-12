#include "pch.h"
#include "./LevelData.h"
#include "./TileSet.h"
#include "COM/ComAlloc.h"
#include "Data/Compression.h"
#include "Data/Data.h"
#include "Data/DataWriterReader.h"
#include "Dict/Dict.h"
#include "Graph/Render/RenderAnimation.h"
#include "Module/ModuleFactory.h"
#include "Resource/ResourceValue.h"
#include "String/StringUtil.h"
#include "Value/Values.h"

static const int LEVEL_CACHE_VERSION = 1;

static ff::StaticString RES_COMPRESSION(L"compression");
static ff::StaticString RES_DATA(L"data");
static ff::StaticString RES_ENCODING(L"encoding");
static ff::StaticString RES_FIRSTGID(L"firstgid");
static ff::StaticString RES_HEIGHT(L"height");
static ff::StaticString RES_LAYERS(L"layers");
static ff::StaticString RES_NAME(L"name");
static ff::StaticString RES_OBJECTS(L"objects");
static ff::StaticString RES_OPACITY(L"opacity");
static ff::StaticString RES_ORIENTATION(L"orientation");
static ff::StaticString RES_PLAYERSTART(L"playerstart");
static ff::StaticString RES_POSITION(L"position");
static ff::StaticString RES_RENDERORDER(L"renderorder");
static ff::StaticString RES_SOURCE(L"source");
static ff::StaticString RES_SIZE(L"size");
static ff::StaticString RES_TILEHEIGHT(L"tileheight");
static ff::StaticString RES_TILESET(L"tileset");
static ff::StaticString RES_TILESETS(L"tilesets");
static ff::StaticString RES_TILESIZE(L"tilesize");
static ff::StaticString RES_TILEWIDTH(L"tilewidth");
static ff::StaticString RES_TYPE(L"type");
static ff::StaticString RES_VERSION(L"version");
static ff::StaticString RES_VISIBLE(L"visible");
static ff::StaticString RES_WIDTH(L"width");
static ff::StaticString RES_X(L"x");
static ff::StaticString RES_Y(L"y");

struct LevelDataInternal
{
	struct Layer
	{
		ff::String _name;
		ff::PointInt _size;
		ff::PointFixedInt _position;
		ff::ComPtr<ff::IData> _data;
		ff::FixedInt _opacity;
		bool _visible;
	};

	struct TileSet
	{
		ff::TypedResource<TileSetResource> _tileSet;
		size_t _firstId;
	};

	ff::String _name;
	ff::PointInt _size;
	ff::PointFixedInt _tileSize;
	ff::PointFixedInt _playerStart;
	ff::Vector<Layer> _layers;
	ff::Vector<TileSet> _tileSets;
};

BEGIN_INTERFACES(LevelDataResource)
	HAS_INTERFACE(ff::IResourcePersist)
END_INTERFACES()

static ff::ModuleStartup Register([](ff::Module& module)
	{
		module.RegisterClassT<LevelDataResource>(L"levelData");
	});

LevelDataResource::LevelDataResource()
{
}

LevelDataResource::~LevelDataResource()
{
}

bool LevelDataResource::IsLevelDataFullyLoaded() const
{
	assertRetVal(_levelData != nullptr, false);

	for (const LevelDataInternal::TileSet& tileSet : _levelData->_tileSets)
	{
		auto tileSetResource = tileSet._tileSet;
		noAssertRetVal(tileSetResource.HasObject(), false);
	}

	return true;
}

LevelData LevelDataResource::GetLevelData() const
{
	LevelData loadedData;
	assertRetVal(IsLevelDataFullyLoaded(), loadedData);

	loadedData._name = _levelData->_name;
	loadedData._size = _levelData->_size;
	loadedData._tileSize = _levelData->_tileSize;
	loadedData._playerStart = _levelData->_playerStart;

	for (const LevelDataInternal::TileSet& tileSetData : _levelData->_tileSets)
	{
		LevelData::TileSet loadedTileSet;
		ff::TypedResource<TileSetResource> tileSetResource = tileSetData._tileSet;
		loadedTileSet._tileSet = std::move(tileSetResource->GetTileSet());
		loadedTileSet._firstId = tileSetData._firstId;
		loadedTileSet._countId = loadedTileSet._tileSet._tiles.Size();

		loadedData._tileSets.Push(std::move(loadedTileSet));
	}

	for (const LevelDataInternal::Layer& layer : _levelData->_layers)
	{
		LevelData::Layer loadedLayer;
		loadedLayer._name = layer._name;
		loadedLayer._size = layer._size;
		loadedLayer._position = layer._position;
		loadedLayer._opacity = layer._opacity;
		loadedLayer._visible = layer._visible;

		loadedLayer._tiles.Resize(layer._size.x * layer._size.y);
		const Tile** outTile = loadedLayer._tiles.Data();
		const unsigned int* ids = (const unsigned int*)layer._data->GetMem();

		for (size_t i = 0; i < loadedLayer._tiles.Size(); i++, outTile++, ids++)
		{
			*outTile = nullptr;

			size_t id = *ids;
			if (id)
			{
				for (const LevelData::TileSet& tileSet : loadedData._tileSets)
				{
					if (id >= tileSet._firstId && id < tileSet._firstId + tileSet._countId)
					{
						*outTile = &tileSet._tileSet._tiles[id - tileSet._firstId];
						break;
					}
				}

				assert(*outTile);
			}
		}

		loadedData._layers.Push(std::move(loadedLayer));
	}

	return loadedData;
}

bool LevelDataResource::LoadFromSource(const ff::Dict& dict)
{
	assertRetVal(dict.Get<ff::StringValue>(RES_TYPE) == L"map", false);
	assertRetVal(dict.Get<ff::StringValue>(RES_ORIENTATION) == L"orthogonal", false);
	assertRetVal(dict.Get<ff::StringValue>(RES_RENDERORDER) == L"right-down", false);

	_levelData = std::make_unique<LevelDataInternal>();
	_levelData->_tileSize.x = dict.Get<ff::FixedIntValue>(RES_TILEWIDTH);
	_levelData->_tileSize.y = dict.Get<ff::FixedIntValue>(RES_TILEHEIGHT);
	_levelData->_size.x = dict.Get<ff::IntValue>(RES_WIDTH);
	_levelData->_size.y = dict.Get<ff::IntValue>(RES_HEIGHT);
	_levelData->_playerStart = ff::PointFixedInt::Zeros();
	assertRetVal(_levelData->_size.x * _levelData->_size.y > 0, false);

	ff::Vector<ff::ValuePtr> layers = dict.Get<ff::ValueVectorValue>(RES_LAYERS);
	ff::Vector<ff::ValuePtr> tileSets = dict.Get<ff::ValueVectorValue>(RES_TILESETS);

	ff::ComPtr<ff::IResourceLoadListener> loadListener;
	assertRetVal(loadListener.QueryFrom(dict.Get<ff::ObjectValue>(ff::RES_LOAD_LISTENER)), false);

	for (ff::ValuePtr layerValue : layers)
	{
		ff::ValuePtr layerDictValue = layerValue->Convert<ff::DictValue>();
		if (layerDictValue)
		{
			const ff::Dict& layerDict = layerDictValue->GetValue<ff::DictValue>();
			if (layerDict.Get<ff::StringValue>(RES_TYPE) == L"tilelayer")
			{
				assertRetVal(LoadTileLayerFromSource(layerDict), false);
			}
			else if (layerDict.Get<ff::StringValue>(RES_TYPE) == L"objectgroup")
			{
				assertRetVal(LoadObjectLayerFromSource(layerDict), false);
			}
			else
			{
				assertRetVal(false, false);
			}
		}
	}

	for (ff::ValuePtr tileSetValue : tileSets)
	{
		ff::ValuePtr tileSetDictValue = tileSetValue->Convert<ff::DictValue>();
		if (tileSetDictValue)
		{
			assertRetVal(LoadTileSetFromSource(loadListener, tileSetDictValue->GetValue<ff::DictValue>()), false);
		}
	}

	return true;
}

bool LevelDataResource::LoadFromCache(const ff::Dict& dict)
{
	int version = dict.Get<ff::IntValue>(RES_VERSION);
	assertRetVal(version == ::LEVEL_CACHE_VERSION, false);

	_levelData = std::make_unique<LevelDataInternal>();
	_levelData->_name = dict.Get<ff::StringValue>(RES_NAME);
	_levelData->_size = dict.Get<ff::PointIntValue>(RES_SIZE);
	_levelData->_tileSize = dict.Get<ff::PointFixedIntValue>(RES_TILESIZE);
	_levelData->_playerStart = dict.Get<ff::PointFixedIntValue>(RES_PLAYERSTART);

	ff::Vector<ff::ValuePtr> layers = dict.Get<ff::ValueVectorValue>(RES_LAYERS);
	ff::Vector<ff::ValuePtr> tileSets = dict.Get<ff::ValueVectorValue>(RES_TILESETS);

	for (ff::ValuePtr value : layers)
	{
		assertRetVal(LoadTileLayerFromCache(value->GetValue<ff::DictValue>()), false);
	}

	for (ff::ValuePtr value : tileSets)
	{
		assertRetVal(LoadTileSetFromCache(value->GetValue<ff::DictValue>()), false);
	}

	return true;
}

bool LevelDataResource::SaveToCache(ff::Dict& dict)
{
	assertRetVal(_levelData != nullptr, false);

	dict.Set<ff::IntValue>(RES_VERSION, ::LEVEL_CACHE_VERSION);
	dict.Set<ff::StringValue>(RES_NAME, _levelData->_name);
	dict.Set<ff::PointIntValue>(RES_SIZE, _levelData->_size);
	dict.Set<ff::PointFixedIntValue>(RES_TILESIZE, _levelData->_tileSize);
	dict.Set<ff::PointFixedIntValue>(RES_PLAYERSTART, _levelData->_playerStart);

	ff::Vector<ff::ValuePtr> layers;
	for (LevelDataInternal::Layer& layerData : _levelData->_layers)
	{
		ff::Dict layerDict;
		layerDict.Set<ff::StringValue>(RES_NAME, layerData._name);
		layerDict.Set<ff::PointIntValue>(RES_SIZE, layerData._size);
		layerDict.Set<ff::PointFixedIntValue>(RES_POSITION, layerData._position);
		layerDict.Set<ff::DataValue>(RES_DATA, layerData._data);
		layerDict.Set<ff::FixedIntValue>(RES_OPACITY, layerData._opacity);
		layerDict.Set<ff::BoolValue>(RES_VISIBLE, layerData._visible);

		layers.Push(ff::Value::New<ff::DictValue>(std::move(layerDict)));
	}

	ff::Vector<ff::ValuePtr> tileSets;
	for (LevelDataInternal::TileSet& tileSet : _levelData->_tileSets)
	{
		ff::Dict tileSetDict;
		tileSetDict.Set<ff::SharedResourceWrapperValue>(RES_TILESET, tileSet._tileSet.GetResourceValue());
		tileSetDict.Set<ff::SizeValue>(RES_FIRSTGID, tileSet._firstId);

		tileSets.Push(ff::Value::New<ff::DictValue>(std::move(tileSetDict)));
	}

	dict.SetValue(RES_LAYERS, ff::Value::New<ff::ValueVectorValue>(std::move(layers)));
	dict.SetValue(RES_TILESETS, ff::Value::New<ff::ValueVectorValue>(std::move(tileSets)));

	return true;
}

bool LevelDataResource::LoadTileLayerFromSource(const ff::Dict& dict)
{
	LevelDataInternal::Layer layerData;

	assertRetVal(dict.Get<ff::StringValue>(RES_COMPRESSION) == L"zlib", false);
	assertRetVal(dict.Get<ff::StringValue>(RES_ENCODING) == L"base64", false);

	layerData._name = dict.Get<ff::StringValue>(RES_NAME);
	layerData._size.x = dict.Get<ff::IntValue>(RES_WIDTH);
	layerData._size.y = dict.Get<ff::IntValue>(RES_HEIGHT);
	layerData._position.x = dict.Get<ff::FixedIntValue>(RES_X);
	layerData._position.y = dict.Get<ff::FixedIntValue>(RES_Y);
	layerData._opacity = dict.Get<ff::FixedIntValue>(RES_OPACITY);
	layerData._visible = dict.Get<ff::BoolValue>(RES_VISIBLE);

	ff::String dataString = dict.Get<ff::StringValue>(RES_DATA);
	assertRetVal(dataString.size(), false);

	ff::ComPtr<ff::IData> dataCompressed = ff::DecodeBase64(dataString);
	assertRetVal(dataCompressed, false);

	ff::ComPtr<ff::IDataReader> dataCompressedReader;
	assertRetVal(ff::CreateDataReader(dataCompressed, 0, &dataCompressedReader), false);

	ff::ComPtr<ff::IDataVector> dataVector;
	ff::ComPtr<ff::IDataWriter> dataWriter;
	assertRetVal(ff::CreateDataWriter(&dataVector, &dataWriter), false);
	dataVector->GetVector().Reserve(layerData._size.x * layerData._size.y * sizeof(int));

	assertRetVal(ff::UncompressData(dataCompressedReader, dataCompressed->GetSize(), dataWriter), false);
	layerData._data = dataVector;

	_levelData->_layers.Push(std::move(layerData));

	return true;
}

bool LevelDataResource::LoadObjectLayerFromSource(const ff::Dict& dict)
{
	ff::PointFixedInt layerOffset(dict.Get<ff::FixedIntValue>(RES_X), dict.Get<ff::FixedIntValue>(RES_Y));
	ff::Vector<ff::ValuePtr> objects = dict.Get<ff::ValueVectorValue>(RES_OBJECTS);

	for (ff::ValuePtr objectValue : objects)
	{
		ff::ValuePtr objectDictValue = objectValue->Convert<ff::DictValue>();
		if (objectDictValue)
		{
			const ff::Dict& objectDict = objectDictValue->GetValue<ff::DictValue>();
			ff::String type = objectDict.Get<ff::StringValue>(RES_TYPE);

			if (type == L"Player")
			{
				_levelData->_playerStart = ff::PointFixedInt(
					objectDict.Get<ff::FixedIntValue>(RES_X),
					objectDict.Get<ff::FixedIntValue>(RES_Y)) + layerOffset;
			}
		}
	}

	return true;
}

bool LevelDataResource::LoadTileSetFromSource(ff::IResourceLoadListener* loadListener, const ff::Dict& dict)
{
	LevelDataInternal::TileSet tileSet;
	tileSet._firstId = dict.Get<ff::SizeValue>(RES_FIRSTGID);
	assertRetVal(tileSet._firstId > 0, false);

	ff::String fullSource = dict.Get<ff::StringValue>(RES_SOURCE);
	ff::String name = ff::GetPathTail(fullSource);
	ff::ChangePathExtension(name, ff::GetEmptyString());
	assertRetVal(name.size(), false);

	tileSet._tileSet.Init(loadListener->AddResourceReference(name));
	_levelData->_tileSets.Push(std::move(tileSet));

	return true;
}

bool LevelDataResource::LoadTileLayerFromCache(const ff::Dict& dict)
{
	LevelDataInternal::Layer layerData;
	layerData._name = dict.Get<ff::StringValue>(RES_NAME);
	layerData._size = dict.Get<ff::PointIntValue>(RES_SIZE);
	layerData._position = dict.Get<ff::PointFixedIntValue>(RES_POSITION);
	layerData._data = dict.Get<ff::DataValue>(RES_DATA);
	layerData._opacity = dict.Get<ff::FixedIntValue>(RES_OPACITY);
	layerData._visible = dict.Get<ff::BoolValue>(RES_VISIBLE);

	_levelData->_layers.Push(std::move(layerData));

	return true;
}

bool LevelDataResource::LoadTileSetFromCache(const ff::Dict& dict)
{
	LevelDataInternal::TileSet tileSet;

	tileSet._tileSet.Init(dict.Get<ff::SharedResourceWrapperValue>(RES_TILESET));
	assertRetVal(tileSet._tileSet.GetResourceValue() != nullptr, false);

	tileSet._firstId = dict.Get<ff::SizeValue>(RES_FIRSTGID);
	assertRetVal(tileSet._firstId > 0, false);

	_levelData->_tileSets.Push(std::move(tileSet));

	return true;
}
