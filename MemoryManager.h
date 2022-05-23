#pragma once
//#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <functional>
#include <vector>
#include <string>
#include "Memory.h"
#include "MemoryAlgorithms.h"

class MemoryManager {
public:
	MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator);
	~MemoryManager();
	void initialize(size_t sizeInWords);
	void shutdown();
	void* allocate(size_t sizeInBytes);
	void free(void* address);
	void setAllocator(std::function<int(int, void*)> allocator);
	int dumpMemoryMap(char* filename);
	void* getList();
	void* getBitmap();
	unsigned getWordSize();
	void* getMemoryStart();
	unsigned getMemoryLimit();
	unsigned int BinaryConvertor(std::string& byte);
	char* getBuffer(unsigned int& bufferSize);
private:
	unsigned capacity;
	unsigned wordSize;
	Memory memory;
	std::function<int(int, void*)> allocator;
};