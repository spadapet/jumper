#include "pch.h"
#include "./Helpers.h"
#include "./TileSet.h"
#include "Data/Data.h"
#include "Data/DataWriterReader.h"
#include "Data/DataPersist.h"
#include "Dict/Dict.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/RenderAnimation.h"
#include "Graph/Texture/Texture.h"
#include "Graph/Sprite/Sprite.h"
#include "Graph/Sprite/SpriteList.h"
#include "Module/ModuleFactory.h"
#include "String/StringUtil.h"
#include "Value/Values.h"

static const int TILE_CACHE_VERSION = 1;

static ff::StaticString RES_COLUMNS(L"columns");
static ff::StaticString RES_HEIGHT(L"height");
static ff::StaticString RES_ID(L"id");
static ff::StaticString RES_IMAGE(L"image");
static ff::StaticString RES_IMAGEHEIGHT(L"imageheight");
static ff::StaticString RES_IMAGEWIDTH(L"imagewidth");
static ff::StaticString RES_MARGIN(L"margin");
static ff::StaticString RES_NAME(L"name");
static ff::StaticString RES_OBJECTGROUP(L"objectgroup");
static ff::StaticString RES_OBJECTS(L"objects");
static ff::StaticString RES_SPACING(L"spacing");
static ff::StaticString RES_SPRITES(L"sprites");
static ff::StaticString RES_TILECOUNT(L"tilecount");
static ff::StaticString RES_TILEHEIGHT(L"tileheight");
static ff::StaticString RES_TILES(L"tiles");
static ff::StaticString RES_TILEWIDTH(L"tilewidth");
static ff::StaticString RES_TYPE(L"type");
static ff::StaticString RES_WIDTH(L"width");
static ff::StaticString RES_VERSION(L"version");
static ff::StaticString RES_X(L"x");
static ff::StaticString RES_Y(L"y");

struct TileSpriteInternal
{
	size_t _index;
	size_t _frames;
};

struct TileInternal
{
	ff::Vector<TileSpriteInternal, 4> _animSprites;
	ff::Vector<TileHitRect, 4> _hitRects;
	TileType _type;
};

struct TileSetInternal
{
	ff::String _name;
	ff::ComPtr<ff::ISpriteList> _sprites;
	ff::Vector<TileInternal> _tiles;
};

BEGIN_INTERFACES(TileSetResource)
	HAS_INTERFACE(ff::IResourcePersist)
END_INTERFACES()

static ff::ModuleStartup Register([](ff::Module& module)
	{
		module.RegisterClassT<TileSetResource>(L"tileSet");
	});

bool TileHitRect::HasAll(TileHitRectType type) const
{
	return Helpers::HasAllFlags(_type, type);
}

bool TileHitRect::HasAny(TileHitRectType type) const
{
	return Helpers::HasAnyFlags(_type, type);
}

bool TileHitRect::HasAny(TileHitRectType type1, TileHitRectType type2) const
{
	return Helpers::HasAnyFlags(_type, Helpers::CombineFlags(type1, type2));
}

bool TileHitRect::IsSolid() const
{
	return HasAll(TileHitRectType::Solid);
}

bool TileHitRect::IsFootRamp() const
{
	return HasAny(TileHitRectType::RampFoot);
}

bool TileHitRect::IsHeadRamp() const
{
	return HasAny(TileHitRectType::RampHead);
}

TileSetResource::TileSetResource()
{
}

TileSetResource::~TileSetResource()
{
	if (_device)
	{
		_device->RemoveChild(this);
	}
}

HRESULT TileSetResource::_Construct(IUnknown* unkOuter)
{
	assertRetVal(_device.QueryFrom(unkOuter), E_INVALIDARG);
	_device->AddChild(this);

	return __super::_Construct(unkOuter);
}

ff::IGraphDevice* TileSetResource::GetDevice() const
{
	return _device;
}

bool TileSetResource::Reset()
{
	return true;
}

TileSet TileSetResource::GetTileSet() const
{
	TileSet tileSet;
	assertRetVal(_tileSet != nullptr, tileSet);

	tileSet._name = _tileSet->_name;

	for (const TileInternal& tileInternal : _tileSet->_tiles)
	{
		Tile tile;
		tile._hitRects = tileInternal._hitRects;
		tile._type = tileInternal._type;

		if (tileInternal._animSprites.Size())
		{
			tile._anim.QueryFrom(_tileSet->_sprites->Get(tileInternal._animSprites[0]._index));
		}

		tileSet._tiles.Push(std::move(tile));
	}

	return tileSet;
}

static TileType ParseTileType(ff::StringRef str)
{
	TileType type = TileType::None;

	for (ff::String part : ff::SplitString(str, ff::String(L","), false))
	{
		ff::StripSpaces(part);

		if (part == L"Solid")
		{
			type = Helpers::SetFlags(type, TileType::Solid);
		}
	}

	return type;
}

static TileHitRectType ParseHitRectType(ff::StringRef str)
{
	TileHitRectType type = TileHitRectType::None;

	for (ff::String part : ff::SplitString(str, ff::String(L","), false))
	{
		ff::StripSpaces(part);

		if (part == L"Solid")
		{
			type = Helpers::SetFlags(type, TileHitRectType::Solid);
		}
		else if (part == L"Water")
		{
			type = Helpers::SetFlags(type, TileHitRectType::Water);
		}
		else if (part == L"RampUp")
		{
			type = Helpers::SetFlags(type, TileHitRectType::RampUp);
		}
		else if (part == L"RampDown")
		{
			type = Helpers::SetFlags(type, TileHitRectType::RampDown);
		}
		else if (part == L"HeadUp")
		{
			type = Helpers::SetFlags(type, TileHitRectType::HeadUp);
		}
		else if (part == L"HeadDown")
		{
			type = Helpers::SetFlags(type, TileHitRectType::HeadDown);
		}
		else if (part == L"TopHeadDown" ||
			part == L"TopHeadUp" ||
			part == L"BottomRampUp" ||
			part == L"BottomRampDown" ||
			part == L"RightRampUp" ||
			part == L"LeftRampDown" ||
			part == L"RightHeadDown" ||
			part == L"LeftHeadUp" ||
			part == L"RampSibling")
		{
			type = Helpers::SetFlags(type, TileHitRectType::RampSibling);
		}
	}

	return type;
}

bool TileSetResource::LoadFromSource(const ff::Dict& dict)
{
	_tileSet = std::make_unique<TileSetInternal>();

	assertRetVal(dict.Get<ff::StringValue>(RES_TYPE) == L"tileset", false);
	assertRetVal(ff::CreateSpriteList(_device, &_tileSet->_sprites), false);
	_tileSet->_name = dict.Get<ff::StringValue>(RES_NAME);

	ff::String textureFile = dict.Get<ff::StringValue>(ff::RES_BASE);
	ff::AppendPathTail(textureFile, dict.Get<ff::StringValue>(RES_IMAGE));
	textureFile = ff::CanonicalizePath(textureFile);

	ff::ComPtr<ff::ITexture> texture = _device->CreateTexture(textureFile, ff::TextureFormat::RGBA32, 1);
	assertRetVal(texture, false);

	ff::ComPtr<ff::IResourceLoadListener> loadListener;
	if (loadListener.QueryFrom(dict.Get<ff::ObjectValue>(ff::RES_LOAD_LISTENER)))
	{
		loadListener->AddFile(textureFile);
	}

	size_t columns = dict.Get<ff::SizeValue>(RES_COLUMNS);
	size_t tileCount = dict.Get<ff::SizeValue>(RES_TILECOUNT);
	ff::FixedInt margin = dict.Get<ff::FixedIntValue>(RES_MARGIN);
	ff::FixedInt spacing = dict.Get<ff::FixedIntValue>(RES_SPACING);
	ff::PointFixedInt tileSize(dict.Get<ff::FixedIntValue>(RES_TILEWIDTH), dict.Get<ff::FixedIntValue>(RES_TILEHEIGHT));
	ff::PointFixedInt pos(margin, margin);

	ff::Map<size_t, ff::Dict> tileDicts;
	for (ff::ValuePtr tileValue : dict.Get<ff::ValueVectorValue>(RES_TILES))
	{
		ff::Dict tileDict = tileValue->GetValue<ff::DictValue>();
		if (tileDict.GetValue(RES_ID))
		{
			size_t id = tileDict.Get<ff::SizeValue>(RES_ID);
			tileDicts.SetKey(id, std::move(tileDict));
		}
	}

	for (size_t i = 0, column = 0; i < tileCount; i++)
	{
		ff::Dict tileDict;
		if (tileDicts.KeyExists(i))
		{
			tileDict = tileDicts.GetKey(i)->GetValue();
		}

		ff::ComPtr<ff::ISprite> sprite = _tileSet->_sprites->Add(
			texture->AsTextureView(),
			ff::String::format_new(L"%lu", i),
			ff::RectFixedInt(pos, pos + tileSize).ToType<float>(),
			ff::PointFloat::Zeros(),
			ff::PointFloat::Ones(),
			ff::SpriteType::Unknown);
		assertRetVal(sprite, false);

		TileSpriteInternal tileSprite;
		tileSprite._index = i;
		tileSprite._frames = 0;

		TileInternal tile;
		tile._animSprites.Push(tileSprite);
		tile._type = ::ParseTileType(tileDict.Get<ff::StringValue>(RES_TYPE));

		// Read hit rects
		ff::Vector<ff::ValuePtr> objectValues = tileDict.Get<ff::DictValue>(RES_OBJECTGROUP).Get<ff::ValueVectorValue>(RES_OBJECTS);
		tile._hitRects.Reserve(objectValues.Size());

		for (ff::ValuePtr objectValue : objectValues)
		{
			ff::ValuePtrT<ff::DictValue> objectDictValue = objectValue;
			if (objectDictValue)
			{
				const ff::Dict& objectDict = objectDictValue.GetValue();
				TileHitRect hitRect;
				hitRect._rect.SetRect(
					objectDict.Get<ff::IntValue>(RES_X),
					objectDict.Get<ff::IntValue>(RES_Y),
					objectDict.Get<ff::IntValue>(RES_WIDTH),
					objectDict.Get<ff::IntValue>(RES_HEIGHT));
				hitRect._rect.right += hitRect._rect.left;
				hitRect._rect.bottom += hitRect._rect.top;
				hitRect._rect = hitRect._rect.Normalize();
				hitRect._type = ::ParseHitRectType(objectDict.Get<ff::StringValue>(RES_TYPE));

				tile._hitRects.Push(hitRect);
			}
		}

		_tileSet->_tiles.Push(std::move(tile));

		if (++column >= columns)
		{
			column = 0;
			pos.x = margin;
			pos.y += tileSize.y + spacing;
		}
		else
		{
			pos.x += tileSize.x + spacing;
		}
	}

	return true;
}

bool TileSetResource::LoadFromCache(const ff::Dict& dict)
{
	int version = dict.Get<ff::IntValue>(RES_VERSION);
	assertRetVal(version == ::TILE_CACHE_VERSION, false);

	_tileSet = std::make_unique<TileSetInternal>();
	_tileSet->_name = dict.Get<ff::StringValue>(RES_NAME);
	size_t tileCount = dict.Get<ff::SizeValue>(RES_TILECOUNT);
	assertRetVal(_tileSet->_sprites.QueryFrom(dict.Get<ff::ObjectValue>(RES_SPRITES)), false);
	ff::ComPtr<ff::IData> tilesData = dict.Get<ff::DataValue>(RES_TILES);
	assertRetVal(tilesData, false);

	ff::ComPtr<ff::IDataReader> tilesReader;
	assertRetVal(ff::CreateDataReader(tilesData, 0, &tilesReader), false);

	for (size_t i = 0; i < tileCount; i++)
	{
		TileInternal tile;
		assertRetVal(ff::LoadData(tilesReader, tile._type), false);

		DWORD count;
		assertRetVal(ff::LoadData(tilesReader, count), false);

		if (count)
		{
			tile._animSprites.Resize(count);
			assertRetVal(ff::LoadBytes(tilesReader, tile._animSprites.Data(), tile._animSprites.ByteSize()), false);
		}

		assertRetVal(ff::LoadData(tilesReader, count), false);

		if (count)
		{
			tile._hitRects.Resize(count);
			assertRetVal(ff::LoadBytes(tilesReader, tile._hitRects.Data(), tile._hitRects.ByteSize()), false);
		}

		_tileSet->_tiles.Push(std::move(tile));
	}

	return true;
}

bool TileSetResource::SaveToCache(ff::Dict& dict)
{
	ff::Dict spritesDict;
	assertRetVal(ff::SaveResourceToCache(_tileSet->_sprites, spritesDict), false);

	ff::ComPtr<ff::IDataWriter> tilesWriter;
	ff::ComPtr<ff::IDataVector> tilesData;
	assertRetVal(ff::CreateDataWriter(&tilesData, &tilesWriter), false);

	for (const TileInternal& tile : _tileSet->_tiles)
	{
		DWORD spriteCount = (DWORD)tile._animSprites.Size();
		DWORD rectCount = (DWORD)tile._hitRects.Size();
		ff::SaveData(tilesWriter, tile._type);
		ff::SaveData(tilesWriter, spriteCount);

		if (spriteCount)
		{
			ff::SaveBytes(tilesWriter, tile._animSprites.ConstData(), tile._animSprites.ByteSize());
		}

		ff::SaveData(tilesWriter, rectCount);

		if (rectCount)
		{
			ff::SaveBytes(tilesWriter, tile._hitRects.ConstData(), tile._hitRects.ByteSize());
		}
	}

	dict.Set<ff::IntValue>(RES_VERSION, ::TILE_CACHE_VERSION);
	dict.Set<ff::StringValue>(RES_NAME, _tileSet->_name);
	dict.Set<ff::SizeValue>(RES_TILECOUNT, _tileSet->_tiles.Size());
	dict.Set<ff::DictValue>(RES_SPRITES, std::move(spritesDict));
	dict.Set<ff::DataValue>(RES_TILES, tilesData);

	return true;
}
