#include "MemoryManager.h"


//_____________________________________________________________________________________________________Memory Manager________________________________________________________________________________________________
//Constructor which sets the wordSize and the allocator function
MemoryManager::MemoryManager(unsigned wordSize, std::function<int(int, void*)> allocator) {
	this->wordSize = wordSize;
	capacity = 0;
	Memory temp(0);
	memory = temp;
	this->allocator = allocator;
}

MemoryManager::~MemoryManager() {
	capacity = 0;
	wordSize = 0;
	memory.Clear();
}

//Creates a free block of memory with a word capacity of sizeInWords, which can hold a byte capacity of sizeInWords*wordSize as long as its below the maximum word size of 65,536
void MemoryManager::initialize(size_t sizeInWords) {
	if (sizeInWords <= 65536) {
		capacity = sizeInWords * wordSize;
		Memory temp(sizeInWords);
		memory = temp;
		memory.AddHead(sizeInWords, false);
	}
}

//Delete the list for shutdown
void MemoryManager::shutdown() {
	capacity = 0;
	memory.Clear();
}

//Allocates memory into any free space left in the memory block
void* MemoryManager::allocate(size_t sizeInBytes) {
	//Convert the size in bytes to wsize in words
	int sizeInWords = sizeInBytes / wordSize;

	//If our memory has a capacity of 0 or we are trying to allocate more memory than we can hold in our block then return nullptr
	if (memory.GetCapacity() == 0 || sizeInWords > memory.GetCapacity()) {
		return nullptr;
	}

	//Otherwise, get the offset of the block to allocate using the allocator
	//If the offset is -1, no free block fo the correct size was found so return nullptr
	else {
		uint16_t* list = static_cast<uint16_t*>(getList());
		int offset = allocator(sizeInWords, list);
		delete[] list;
		if (offset == -1) {
			return nullptr;
		}

		//If the offset is a proper offset, find the corresponding block using its offset
		Memory::Block* block = memory.FindByOffset(offset);

		//If the block is the exact size we need, simply fill it and return the data. 
		if (block->getSize() == sizeInWords) {
			memory.FillBlock(block);
			return block->getData();
		}
		//Otherwise, split the block into the portion to be filled and the portion that remains free and return the data
		else {
			Memory::Block* temp = memory.SplitBlock(block, sizeInWords);
			return temp->getData();
		}
	}
}

//Frees space that is requested
void MemoryManager::free(void* address) {
	//Data is passed in, find the block it corresponds to
	uint64_t* currentAddress = static_cast<uint64_t*>(address);
	Memory::Block* currentBlock = memory.FindByData(currentAddress);
	if (currentBlock != nullptr) {
		if (currentBlock->getUsedStatus()) {
			//Free the block, if the right or left blocks relative to the current block are also free then call the CompactRight or CompactLeft algorithms respectively to compact the space into one large free block
			currentBlock->set_block_status(false);
			if (currentBlock->next != nullptr && !currentBlock->next->getUsedStatus()) {
				currentBlock = memory.CompactRight(currentBlock);
			}
			if (currentBlock->prev != nullptr && !currentBlock->prev->getUsedStatus()) {
				currentBlock = memory.CompactLeft(currentBlock);
			}
		}
	}
}

//Sets the allocator to a new function
void MemoryManager::setAllocator(std::function<int(int, void*)> allocator) {
	this->allocator = allocator;
}

int MemoryManager::dumpMemoryMap(char* filename) {
	//Each file has a unique file number (fd). Open file and get this file number
	int fd;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, mode);

	//If open was unsuccessful, fd is -1
	if (fd == -1) {
		return -1;
	}

	//Get the buffer and bufferSize from getBuffer()
	unsigned int bufferSize = 0;
	char* bufferToWrite = getBuffer(bufferSize);

	//Write our buffer to the file and delete the buffer
	ssize_t ret = write(fd, bufferToWrite, bufferSize);
	delete[] bufferToWrite;

	//Write will return -1 if there is an error when writing
	if (ret == -1) {
		return -1;
	}

	//Close the file, if there is an error it will return -1
	int status = close(fd);
	if (status == -1) {
		return -1;
	}

	//If no errors, then we were successful in writing buffer to file. Return 0
	return 0;
}

void* MemoryManager::getList() {
	//Gets all the holes from our linked list memory manager
	std::vector<std::pair<unsigned int, unsigned int>> v;
	memory.FindFreeBlocks(v);
	//If the size of our vector is 0, we found no holes so return nullptr
	if (v.size() == 0) {
		return nullptr;
	}
	else {
		//Otherwise we found holes. The list we return will be of our (vector size * 2) + 1 
		//The first spot of the list will have the number of holes (size of array), then each index of the vector contains an offset and the size of a hole
		unsigned int length = (v.size() * 2) + 1;
		uint16_t* list = new uint16_t[length];

		//Place size into the list
		unsigned int index = 1;
		list[0] = v.size();

		//Place hole offset and then hole size into the list
		for (int ii = 0; ii < v.size(); ii += 1) {
			list[index] = v.at(ii).first;
			index += 1;
			list[index] = v.at(ii).second;
			index += 1;
		}
		return list;
	}
}


void* MemoryManager::getBitmap() {
	//Gets a representation of our linked list in bits (1 for used and 0 for empty);
	std::vector<int> v;
	memory.BitRepresentation(v);

	//We must look at bytes so loop through increments of 8 bits and store them
	std::vector<unsigned int> byteStream; 
	for (unsigned int ii = 0; ii < v.size(); ii += 8) {
		std::string byte;
		for (unsigned int jj = ii; jj < (ii + 8); jj += 1) {
			if (v.at(jj) == 1) {
				byte.push_back('1');
			}
			else {
				byte.push_back('0');
			}
			if (jj + 1 == v.size()) {
				break;
			}
		}
		//Converts every 8 bits (byte) into a decimal and store it in our byteStream
		unsigned int val = BinaryConvertor(byte);
		byteStream.push_back(val);
	}

	//The size of the bitMap will be the number of bytes plus 2 to represent the size of our bitMap
	unsigned int length = byteStream.size()+2;
	uint8_t* bitMap = new uint8_t[length];

	//The size is represented in 2 bytes, where 1 byte has a decimal value of 255
	//If the size is less than 255, then the first byte represents the size and the second will be 0
	if (byteStream.size() <= 255) {
		bitMap[0] = byteStream.size();
		bitMap[1] = 0;
	}
	//If the size is more than 255, than the first byte is 255 and the second will be the size % 256
	else {
		bitMap[0] = 255;
		bitMap[1] = byteStream.size() % 256;
	}
	//Now that we have stored the size, fill our bitMap with the elements of the byteStream, then return the bitMap
	unsigned int index = 2;
	for (int ii = 0; ii < byteStream.size(); ii += 1) {
		bitMap[index] = byteStream.at(ii);
		index += 1;
	}
	return bitMap;
}

//Returns the wordSize
unsigned MemoryManager::getWordSize() {
	return wordSize;
}

//Finds all the allocated memory and collects its data to place in an array. Returns the data array.
void* MemoryManager::getMemoryStart() {
	return memory.FindFilledBlocks();
}

//Returns the capacity of the list
unsigned MemoryManager::getMemoryLimit() {
	return capacity;
}

unsigned int MemoryManager::BinaryConvertor(std::string& byte) {
	int val = 0;
	int power = 0;
	for (unsigned int ii = 0; ii < byte.size(); ii += 1) {
		if (byte.at(ii) == '0') {
			val += 0;
		}
		else {
			unsigned int temp = 1;
			for (unsigned int jj = 0; jj < power; jj += 1) {
				temp *= 2;
			}
			val += temp;
		}
		power += 1;
	}
	return val; 
}

char* MemoryManager::getBuffer(unsigned int& bufferSize) {
	//Get all the hole offsets and sizes
	std::vector<std::pair<unsigned int, unsigned int>> v;
	memory.FindFreeBlocks(v);

	//Place all hole offsets and sizes into a properly formatted string
	std::string sbuffer;
	for (unsigned int ii = 0; ii < v.size()-1; ii += 1) {
		sbuffer.push_back('[');
		std::string temp = std::to_string(v.at(ii).first);
		sbuffer += temp;
		sbuffer.push_back(',');
		sbuffer.push_back(' ');
		std::string temp2 = std::to_string(v.at(ii).second);
		sbuffer += temp2;
		sbuffer.push_back(']');
		sbuffer.push_back(' ');
		sbuffer.push_back('-');
		sbuffer.push_back(' ');
	}
	sbuffer.push_back('[');
	std::string temp = std::to_string(v.at(v.size()-1).first);
	sbuffer += temp;
	sbuffer.push_back(',');
	sbuffer.push_back(' ');
	std::string temp2 = std::to_string(v.at(v.size()-1).second);
	sbuffer += temp2;
	sbuffer.push_back(']');

	//Convert string to char* buffer
	bufferSize = sbuffer.size();
	char* cbuffer = new char[bufferSize];
	for (unsigned int ii = 0; ii < bufferSize; ii += 1) {
		cbuffer[ii] = sbuffer.at(ii);
	}
	return cbuffer;
}
