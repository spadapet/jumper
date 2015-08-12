#include "pch.h"
#include "Core/Globals.h"
#include "Globals/AppGlobals.h"
#include "Services/RenderQueueService.h"

RenderSort::RenderSort(Rendering::Layer layer)
	: _layer(layer)
	, _shaders(Rendering::Shaders::Normal)
	, _index(0)
{
}

RenderSort::RenderSort(Rendering::Layer layer, Rendering::Shaders shaders)
	: _layer(layer)
	, _shaders(shaders)
	, _index(0)
{
}

RenderQueueService::RenderQueueService()
	: _firstOverlayEntry(_entries.cend())
	, _firstHighResEntry(_entries.cend())
	, _firstHighResOverlayEntry(_entries.cend())
	, _firstScreenResEntry(_entries.cend())
	, _firstScreenResOverlayEntry(_entries.cend())
{
}

RenderQueueService::~RenderQueueService()
{
}

void RenderQueueService::Draw(RenderSort sort, void* cookie, RenderQueueFunc callback)
{
	RenderEntry* entry = _frameMemory.Malloc<RenderEntry>();
	entry->_callback = callback;
	entry->_cookie = cookie;
	entry->_sort._shaders = (unsigned char)sort._shaders;
	entry->_sort._index1 = (unsigned char)((sort._index >> 8) & 0xFF);
	entry->_sort._index0 = (unsigned char)(sort._index & 0xFF);
	entry->_sort._layer = (unsigned char)sort._layer;
	_entries.Push(entry);
}

FrameMemory& RenderQueueService::GetFrameMemory()
{
	return _frameMemory;
}

void RenderQueueService::DoneDrawing()
{
	std::sort(_entries.begin(), _entries.end(),
		[](const RenderEntry* lhs, const RenderEntry* rhs) -> bool
		{
			return lhs->_sort._all < rhs->_sort._all;
		});

	for (_firstOverlayEntry = _entries.cbegin(); _firstOverlayEntry != _entries.cend(); _firstOverlayEntry++)
	{
		const RenderEntry* entry = *_firstOverlayEntry;
		if (entry->_sort._layer >= (unsigned char)Rendering::Layer::FirstOverlay)
		{
			break;
		}
	}

	for (_firstHighResEntry = _firstOverlayEntry; _firstHighResEntry != _entries.cend(); _firstHighResEntry++)
	{
		const RenderEntry* entry = *_firstHighResEntry;
		if (entry->_sort._layer >= (unsigned char)Rendering::Layer::FirstHighRes)
		{
			break;
		}
	}

	for (_firstHighResOverlayEntry = _firstHighResEntry; _firstHighResOverlayEntry != _entries.cend(); _firstHighResOverlayEntry++)
	{
		const RenderEntry* entry = *_firstHighResOverlayEntry;
		if (entry->_sort._layer >= (unsigned char)Rendering::Layer::FirstHighResOverlay)
		{
			break;
		}
	}

	for (_firstScreenResEntry = _firstHighResOverlayEntry; _firstScreenResEntry != _entries.cend(); _firstScreenResEntry++)
	{
		const RenderEntry* entry = *_firstScreenResEntry;
		if (entry->_sort._layer >= (unsigned char)Rendering::Layer::FirstScreenRes)
		{
			break;
		}
	}

	for (_firstScreenResOverlayEntry = _firstScreenResEntry; _firstScreenResOverlayEntry != _entries.cend(); _firstScreenResOverlayEntry++)
	{
		const RenderEntry* entry = *_firstScreenResOverlayEntry;
		if (entry->_sort._layer >= (unsigned char)Rendering::Layer::FirstScreenResOverlay)
		{
			break;
		}
	}
}

void RenderQueueService::RenderLowRes(RenderWrapper& render)
{
	Render(render, _entries.cbegin(), _firstOverlayEntry);
}

void RenderQueueService::RenderLowResOverlays(RenderWrapper& render)
{
	Render(render, _firstOverlayEntry, _firstHighResEntry);
}

void RenderQueueService::RenderHighRes(RenderWrapper& render)
{
	Render(render, _firstHighResEntry, _firstHighResOverlayEntry);
}

void RenderQueueService::RenderHighResOverlays(RenderWrapper& render)
{
	Render(render, _firstHighResOverlayEntry, _firstScreenResEntry);
}

void RenderQueueService::RenderScreenRes(RenderWrapper& render)
{
	Render(render, _firstScreenResEntry, _firstScreenResOverlayEntry);
}

void RenderQueueService::RenderScreenResOverlays(RenderWrapper& render)
{
	Render(render, _firstScreenResOverlayEntry, _entries.cend());
}

bool RenderQueueService::HasLowRes() const
{
	return _entries.cbegin() != _firstOverlayEntry;
}

bool RenderQueueService::HasLowResOverlays() const
{
	return _firstOverlayEntry != _firstHighResEntry;
}

bool RenderQueueService::HasHighRes() const
{
	return _firstHighResEntry != _firstHighResOverlayEntry;
}

bool RenderQueueService::HasHighResOverlays() const
{
	return _firstHighResOverlayEntry != _firstScreenResEntry;
}

bool RenderQueueService::HasScreenRes() const
{
	return _firstScreenResEntry != _firstScreenResOverlayEntry;
}

bool RenderQueueService::HasScreenResOverlays() const
{
	return _firstScreenResOverlayEntry != _entries.cend();
}

void RenderQueueService::Reset()
{
	_entries.Clear();
	_firstOverlayEntry = _entries.cend();
	_firstHighResEntry = _entries.cend();
	_firstHighResOverlayEntry = _entries.cend();
	_firstScreenResEntry = _entries.cend();
	_firstScreenResOverlayEntry = _entries.cend();
	_frameMemory.Reset();
}

void RenderQueueService::Render(RenderWrapper& render, RenderEntryIter beginEntry, RenderEntryIter endEntry)
{
	for (RenderEntryIter i = beginEntry; i != endEntry; i++)
	{
		const RenderEntry* entry = *i;
		entry->_callback(render, entry->_cookie);
	}
}
