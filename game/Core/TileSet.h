#pragma once

#include "Graph/GraphDeviceChild.h"
#include "Resource/ResourcePersist.h"

namespace ff
{
	class IRenderAnimation;
	class ISprite;
	class ISpriteList;
}

struct TileSetInternal;

enum class TileType
{
	None = 0x0000,
	Solid = 0x0001,
};

enum class TileHitRectType
{
	None = 0x0000,
	Solid = 0x0001,
	Water = 0x0002,
	RampUp = 0x0004,
	RampDown = 0x0008,
	HeadUp = 0x0010,
	HeadDown = 0x0020,
	RampSibling = 0x0040,

	RampFoot = RampUp | RampDown,
	RampHead = HeadUp | HeadDown,
};

struct TileHitRect
{
	bool HasAll(TileHitRectType type) const;
	bool HasAny(TileHitRectType type) const;
	bool HasAny(TileHitRectType type1, TileHitRectType type2) const;
	bool IsSolid() const;
	bool IsFootRamp() const;
	bool IsHeadRamp() const;

	ff::RectFixedInt _rect;
	TileHitRectType _type;
};

struct Tile
{
	ff::ComPtr<ff::IRenderAnimation> _anim;
	ff::Vector<TileHitRect, 4> _hitRects;
	TileType _type;
};

struct TileSet
{
	ff::String _name;
	ff::Vector<Tile> _tiles;
};

class __declspec(uuid("37176f1b-ee2f-4563-87b1-8b547e66333e"))
	TileSetResource
	: public ff::ComBase
	, public ff::IResourcePersist
	, public ff::IGraphDeviceChild
{
public:
	DECLARE_HEADER(TileSetResource);

	TileSet GetTileSet() const;

	// IGraphDeviceChild
	virtual HRESULT _Construct(IUnknown* unkOuter) override;
	virtual ff::IGraphDevice* GetDevice() const override;
	virtual bool Reset() override;

	// IResourceSave
	virtual bool LoadFromSource(const ff::Dict& dict) override;
	virtual bool LoadFromCache(const ff::Dict& dict) override;
	virtual bool SaveToCache(ff::Dict& dict) override;

private:
	ff::ComPtr<ff::IGraphDevice> _device;
	std::unique_ptr<TileSetInternal> _tileSet;
};
