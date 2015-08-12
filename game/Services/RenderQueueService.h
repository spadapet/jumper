#pragma once

#include "Core/FrameMemory.h"

namespace Rendering
{
	enum class Layer : unsigned char;
	enum class Shaders : unsigned char;
}

class RenderWrapper;

struct RenderSort
{
	RenderSort(Rendering::Layer layer);
	RenderSort(Rendering::Layer layer, Rendering::Shaders shaders);

	Rendering::Layer _layer;
	Rendering::Shaders _shaders;
	unsigned short _index;
};

typedef void(*RenderQueueFunc)(RenderWrapper& render, void* cookie);

template<typename T>
struct RenderQueueData
{
	typedef void(T::* Callback)(RenderQueueData<T>* data, RenderWrapper& render);

	static void StaticCallback(RenderWrapper& render, void* cookie)
	{
		RenderQueueData<T>* data = (RenderQueueData<T>*)cookie;
		(data->_owner->*data->_callback)(data, render);
	}

	T* _owner;
	Callback _callback;
};

template<typename T>
struct RenderQueueNoData
{
	typedef void(T::* Callback)(RenderWrapper& render);

	static void StaticCallback(RenderWrapper& render, void* cookie)
	{
		RenderQueueNoData<T>* data = (RenderQueueNoData<T>*)cookie;
		(data->_owner->*data->_callback)(render);
	}

	T* _owner;
	Callback _callback;
};

class IRenderQueueService
{
public:
	virtual void Draw(RenderSort sort, void* cookie, RenderQueueFunc callback) = 0;
	virtual FrameMemory& GetFrameMemory() = 0;

	// Fast-allocates an object of type T and returns it
	template<typename T>
	T* Draw(RenderSort sort, RenderQueueFunc callback)
	{
		T* cookie = GetFrameMemory().Malloc<T>();
		Draw(sort, cookie, callback);
		return cookie;
	}

	// Fast-allocates an object of type T and calls back on object "owner"
	template<typename C, typename T>
	T* Draw(RenderSort sort, C* owner, typename RenderQueueData<C>::Callback callback)
	{
		RenderQueueData<C>* cookie = Draw<T>(sort, RenderQueueData<C>::StaticCallback);
		cookie->_owner = owner;
		cookie->_callback = callback;
		return static_cast<T*>(cookie);
	}

	// Just calls back on object "owner" without any context data
	template<typename C>
	void Draw(RenderSort sort, C* owner, typename RenderQueueNoData<C>::Callback callback)
	{
		RenderQueueNoData<C>* cookie = Draw<RenderQueueNoData<C>>(sort, RenderQueueNoData<C>::StaticCallback);
		cookie->_owner = owner;
		cookie->_callback = callback;
	}
};

class RenderQueueService : public IRenderQueueService
{
public:
	RenderQueueService();
	~RenderQueueService();

	virtual void Draw(RenderSort sort, void* cookie, RenderQueueFunc callback) override;
	virtual FrameMemory& GetFrameMemory() override;

	void DoneDrawing();

	void RenderLowRes(RenderWrapper& render);
	void RenderLowResOverlays(RenderWrapper& render);
	void RenderHighRes(RenderWrapper& render);
	void RenderHighResOverlays(RenderWrapper& render);
	void RenderScreenRes(RenderWrapper& render);
	void RenderScreenResOverlays(RenderWrapper& render);

	bool HasLowRes() const;
	bool HasLowResOverlays() const;
	bool HasHighRes() const;
	bool HasHighResOverlays() const;
	bool HasScreenRes() const;
	bool HasScreenResOverlays() const;

	void Reset();

private:
	struct RenderEntry
	{
		RenderQueueFunc _callback;
		void* _cookie;

		union
		{
			struct
			{
				unsigned char _shaders;
				unsigned char _index1;
				unsigned char _index0;
				unsigned char _layer;
			};

			unsigned int _all;
		} _sort;
	};

	typedef ff::Vector<const RenderEntry*>::const_iterator RenderEntryIter;
	void Render(RenderWrapper& render, RenderEntryIter beginEntry, RenderEntryIter endEntry);

	ff::Vector<const RenderEntry*> _entries;
	RenderEntryIter _firstOverlayEntry;
	RenderEntryIter _firstHighResEntry;
	RenderEntryIter _firstHighResOverlayEntry;
	RenderEntryIter _firstScreenResEntry;
	RenderEntryIter _firstScreenResOverlayEntry;
	FrameMemory _frameMemory;
};
