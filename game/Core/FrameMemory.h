#pragma once

class FrameMemory
{
public:
	FrameMemory();
	FrameMemory(size_t chunkSize);
	~FrameMemory();

	void* Malloc(size_t bytes, size_t align = __alignof(void*));
	void Reset();

	template<typename T>
	T* Malloc()
	{
		return (T*)Malloc(sizeof(T), __alignof(T));
	}

private:
	FrameMemory(const FrameMemory& rhs) = delete;
	FrameMemory& operator=(const FrameMemory& rhs) = delete;

	char* _pos;
	char* _end;
	size_t _nextChunk;
	size_t _chunkSize;
	ff::Vector<char*> _chunks;
	ff::Vector<char*> _tempChunks;
};
