#include "pch.h"
#include "Core/FrameMemory.h"

FrameMemory::FrameMemory()
	: FrameMemory(0x10000)
{
}

FrameMemory::FrameMemory(size_t chunkSize)
	: _pos(nullptr)
	, _end(nullptr)
	, _nextChunk(0)
	, _chunkSize(chunkSize)
{
}

FrameMemory::~FrameMemory()
{
	Reset();

	for (char* i : _chunks)
	{
		::free(i);
	}
}

static char* Align(char* pos, size_t align)
{
	size_t off = (size_t)pos % align;
	if (off != 0)
	{
		pos += align - off;
	}

	return pos;
}

void* FrameMemory::Malloc(size_t bytes, size_t align)
{
	char* mem = ::Align(_pos, align);

	if (mem + bytes > _end)
	{
		if (bytes > _chunkSize)
		{
			_tempChunks.Push((char*)::malloc(bytes));
			return _tempChunks.GetLast();
		}
		else
		{
			if (_nextChunk == _chunks.Size())
			{
				_chunks.Push((char*)::malloc(_chunkSize));
			}

			_pos = _chunks[_nextChunk++];
			_end = _pos + _chunkSize;
			mem = _pos;
		}
	}

	_pos = mem + bytes;

	return mem;
}

void FrameMemory::Reset()
{
	_pos = nullptr;
	_end = nullptr;
	_nextChunk = 0;

	for (char* i : _tempChunks)
	{
		::free(i);
	}

	_tempChunks.Clear();
}
