#pragma once
#include <vector>
#include <stdint.h>

class Memory {
public:
	//Each block stores if it is free or allocated (used), the offset and size, and the allocated data (If applicable)
	struct Block {
		bool used;
		unsigned int size;
		unsigned int offset;
		Block* next;
		Block* prev;
		uint64_t* data;

		//__________________Constructor________________________
		Block(unsigned int size, bool used, unsigned int offset);

		//___________Modifiers_______________
		void ResetSize(unsigned int size);
		void ResetOffset(unsigned int offset);
		void set_block_status(bool used);

		//_________Getters__________
		unsigned int getSize();
		unsigned int getOffset();
		bool getUsedStatus();
		uint64_t* getData();
	};

	//___________Constructors and Destructors______________
	Memory();
	Memory(unsigned int capacity);
	Memory(const Memory& rhs);
	Memory& operator=(const Memory& rhs);
	~Memory();
	void Clear();

	//___________Adding Memory Blocks to List_____________-
	void AddHead(const unsigned int& size, bool used);
	Block* SplitBlock(Block* blockToSplit, unsigned int size);

	//___________Compacting Algorithms to Free Space____________
	Block* CompactLeft(Block* blockToCompact);
	Block* CompactRight(Block* blockToCompact);

	//____________Algorithms to Find Blocks/Bits of List____________
	Block* FindByOffset(const unsigned int& offset);
	Block* FindByData(const uint64_t* dataToFind);
	void FindFreeBlocks(std::vector<std::pair<unsigned int, unsigned int>>& v);
	uint64_t* FindFilledBlocks();
	void BitRepresentation(std::vector<int>& v);

	//____________Getters_____________
	unsigned int GetCapacity();

	//____________Modifiers___________
	void FillBlock(Block* blockToFill);
	
private:
	uint64_t* listData;
	unsigned int listSize;
	Block* head;
	Block* tail;
	unsigned int memory_capacity;
};
