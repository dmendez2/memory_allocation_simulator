#include "Memory.h"
//______________________________________________________________________________________________________Memory Blocks______________________________________________________________________________________________________
//Constructor which initializes all block variables
Memory::Block::Block(unsigned int size, bool used, unsigned int offset) {
	this->used = used;
	this->size = size;
	this->offset = offset;
	next = nullptr;
	prev = nullptr;
	data = new uint64_t[size];
}

//Delete the data array to reset its size
void Memory::Block::ResetSize(unsigned int size) {
	delete[] data;
	this->size = size;
	data = new uint64_t[size];
}

//The following functions are all simple modifiers and getters
void Memory::Block::ResetOffset(unsigned int offset) {
	this->offset = offset; 
}

void Memory::Block::set_block_status(bool used) {
	this->used = used;
}

unsigned int Memory::Block::getSize() {
	return size;
}

unsigned int Memory::Block::getOffset() {
	return offset;
}

bool Memory::Block::getUsedStatus() {
	return used;
}

uint64_t* Memory::Block::getData() {
	return data;
}

//_______________________________________________________________________________________________________Memory______________________________________________________________________________________________________________
//Default constructor, initializes all pointers to nullptr and all variables to 0
Memory::Memory() {
	head = nullptr;
	tail = nullptr;
	memory_capacity = 0;
	listSize = 0;
	listData = nullptr;
}

//Constructor which initializes only the capacity of the list
Memory::Memory(unsigned int capacity) {
	head = nullptr;
	tail = nullptr;
	memory_capacity = capacity;
	listSize = 0;
	listData = nullptr;
}

//Copy constructor which copies all elements of the rhs list upon creation of the lhs list
Memory::Memory(const Memory& rhs) {
	memory_capacity = rhs.memory_capacity;
	Block* oldCurrent = rhs.head;
	head = new Block(oldCurrent->size, oldCurrent->used, oldCurrent->offset);
	Block* newCurrent = head;
	while (oldCurrent->next != nullptr) {
		oldCurrent = oldCurrent->next;
		Block* temp = new Block(oldCurrent->size, oldCurrent->used, oldCurrent->offset);
		newCurrent->next = temp;
		temp->prev = newCurrent;
		newCurrent = newCurrent->next;
	}
	tail = newCurrent;

	if (rhs.listData == nullptr) {
		listSize = rhs.listSize;
		listData = nullptr;
	}
	else {
		listSize = rhs.listSize;
		listData = new uint64_t[listSize];
		for (unsigned int ii = 0; ii < listSize; ii += 1) {
			listData[ii] = rhs.listData[ii];
		}
	}
}

//Overloaded copy assignment operator which clears the lhs list and copies the rhs list into it
Memory& Memory::operator=(const Memory& rhs) {
	//Deleting all elements from this list so we have a fresh slate to copy the rhs list
	this->Clear();
	if (rhs.head == nullptr) {
		head = nullptr;
		tail = nullptr; 
		memory_capacity = rhs.memory_capacity;
		return *this;
	}
	else {
		//Old current points to the head of the list we want to copy
		Block* oldCurrent = rhs.head;
		//To create a deep copy, we set head equal to a new node we create 
		//And copy the value from the old list
		head = new Block(oldCurrent->size, oldCurrent->used, oldCurrent->offset);
		Block* newCurrent = head;
		//We will now iterate through the old list
		while (oldCurrent->next != nullptr) {
			//old current points to next element in the old list
			oldCurrent = oldCurrent->next;
			//We now create a new node with the value from the old list
			Block* temp = new Block(oldCurrent->size, oldCurrent->used, oldCurrent->offset);
			//This connects the new node to the new list
			newCurrent->next = temp;
			temp->prev = newCurrent;
			//Move over 1 element in the new list
			newCurrent = newCurrent->next;
		}
		//Make sure to set tail of new list at the last node
		tail = newCurrent;
		memory_capacity = rhs.memory_capacity;


		if (rhs.listData == nullptr) {
			listSize = rhs.listSize;
			listData = nullptr;
		}
		else {
			listSize = rhs.listSize;
			listData = new uint64_t[listSize];
			for (unsigned int ii = 0; ii < listSize; ii += 1) {
				listData[ii] = rhs.listData[ii];
			}
		}

		return *this;
	}
}

//Destructor which deletes each block and each block's data in the list and resets all variables to 0 (Called when list falls out of scope)
Memory::~Memory() {
	Clear();
}

//A clear function to delete lists which is exactly the same as the destructor but can be called anytime in the lifetime of the list (Useful for deleting and reassigning lists)
void Memory::Clear() {
	if (head == nullptr) {
		head = nullptr;
		tail = nullptr;
	}
	else {
		Block* current = head;
		while (current->next != nullptr) {
			head = head->next;
			delete[] current->data;
			delete current;
			current = head;
		}
		delete[] head->data;
		delete head;
		head = nullptr;
		tail = nullptr;
	}

	if (listData != nullptr) {
		delete[] listData; 
		listData = nullptr;
	}

	memory_capacity = 0;
	listSize = 0;
}

//Function to add a block of memory to the front of the list
void Memory::AddHead(const unsigned int& size, bool used) {
	//If head is nullptr, this is the first element in our list so create a new block and assign the head and tail to it
	if (head == nullptr) {
		head = new Block(size, used, 0);
		tail = head;
	}
	//Otherwise create a new block, assign head to it, the next block is the old head
	else {
		Block* temp = new Block(size, used, 0);
		temp->next = head;
		head->prev = temp;
		head = temp;
	}
}

//If a free block is called to be allocated and it has extra room, split the block into a free part and a used part
Memory::Block* Memory::SplitBlock(Block* blockToSplit, unsigned int size) {
	//Size of the free block will be totalBlockSize - sizeToBeAllocated
	//The offset of the free block will be the current offset plus the size of what will be allocated
	unsigned int newSize = blockToSplit->size - size;
	unsigned int newOffset = blockToSplit->offset + size;
	unsigned int oldOffset = blockToSplit->offset;

	//If this is the head, reset the size and offset. Then add a head of the size to be allocated
	if (head == blockToSplit) {
		blockToSplit->ResetSize(newSize);
		blockToSplit->ResetOffset(newOffset);
		AddHead(size, true);
		return head; 
	}

	//If this is the tail, the new allocated block will be placed between the resized tail and the tail's previous block
	else if (tail == blockToSplit) {
		blockToSplit->ResetSize(newSize);
		blockToSplit->ResetOffset(newOffset);
		Block* temp = tail->prev;

		Block* newFilledBlock = new Block(size, true, oldOffset);
		temp->next = newFilledBlock;
		newFilledBlock->prev = temp;

		newFilledBlock->next = tail;
		tail->prev = newFilledBlock;

		return newFilledBlock;
	}

	//If this is a general block, then place the new allocated block between the current block after its resized and the previous block
	else {
		blockToSplit->ResetSize(newSize);
		blockToSplit->ResetOffset(newOffset);
		Block* temp = blockToSplit->prev;

		Block* newFilledBlock = new Block(size, true, oldOffset);
		temp->next = newFilledBlock;
		newFilledBlock->prev = temp;

		newFilledBlock->next = blockToSplit;
		temp->prev = newFilledBlock;

		return newFilledBlock;
	}
}

//If a block is freed and the block to the left is also free, these are compacted into one large free block
Memory::Block* Memory::CompactLeft(Memory::Block* blockToCompact) {
	//Size of the compacted block is the two blocks sizes together, the offset is the offset of the leftmost block
	unsigned int newSize = blockToCompact->getSize() + blockToCompact->prev->getSize();
	unsigned int newOffset = blockToCompact->prev->getOffset();
	Memory::Block* newBlock = new Block(newSize, false, newOffset);

	//In all the following cases, delete the current block and the block to the left. The new compacted block will take up the space these blocks used to reside in.
	//General case, delete both blocks and their data. Then connect the new compacted block to the old neighbors of the blocks that were compacted
	if (blockToCompact->prev->prev != nullptr && blockToCompact->next != nullptr) {
		Memory::Block* tempLeft = blockToCompact->prev->prev;
		Memory::Block* tempRight = blockToCompact->next;

		delete[] blockToCompact->prev->data;
		delete[] blockToCompact->data;
		delete blockToCompact->prev;
		delete blockToCompact;

		newBlock->prev = tempLeft;
		newBlock->next = tempRight;
		tempLeft->next = newBlock;
		tempRight->prev = newBlock;
		return newBlock;
	}

	//If these were the only two blocks in the list, then compact them and the compacted block is the new head and tail
	else if (blockToCompact->prev->prev == nullptr && blockToCompact->next == nullptr) {
		delete[] blockToCompact->prev->data;
		delete[] blockToCompact->data;
		delete blockToCompact->prev;
		delete blockToCompact;
		head = newBlock;
		tail = newBlock;
		return newBlock;
	}

	//If the two blocks are the leftmost blocks in the list then they become the new head
	else if (blockToCompact->prev->prev == nullptr) {
		Memory::Block* tempRight = blockToCompact->next;

		delete[] blockToCompact->prev->data;
		delete[] blockToCompact->data;
		delete blockToCompact->prev;
		delete blockToCompact;

		newBlock->next = tempRight;
		tempRight->prev = newBlock;

		head = newBlock;
		return newBlock;
	}

	//If the two blocks are the rightmost blocks in the list then they become the new tail
	else if (blockToCompact->next == nullptr) {
		Memory::Block* tempLeft = blockToCompact->prev->prev;

		delete[] blockToCompact->prev->data;
		delete[] blockToCompact->data;
		delete blockToCompact->prev;
		delete blockToCompact;

		newBlock->prev = tempLeft;
		tempLeft->next = newBlock;

		tail = newBlock;
		return newBlock;
	}
	return nullptr;
}

//If a block is freed and the block to the right is also free, these are compacted into one large free block
Memory::Block* Memory::CompactRight(Memory::Block* blockToCompact) {
	//Size of the compacted block is the two blocks sizes together, the offset is the offset of the leftmost block
	unsigned int newSize = blockToCompact->getSize() + blockToCompact->next->getSize();
	unsigned int newOffset = blockToCompact->getOffset();
	Memory::Block* newBlock = new Block(newSize, false, newOffset);

	//In all the following cases, delete the current block and the block to the right. The new compacted block will take up the space these blocks used to reside in.
	//General case, delete both blocks and their data. Then connect the new compacted block to the old neighbors of the blocks that were compacted
	if (blockToCompact->next->next != nullptr && blockToCompact->prev != nullptr) {
		Memory::Block* tempLeft = blockToCompact->prev;
		Memory::Block* tempRight = blockToCompact->next->next;

		delete[] blockToCompact->next->data;
		delete[] blockToCompact->data;
		delete blockToCompact->next;
		delete blockToCompact;


		newBlock->prev = tempLeft;
		newBlock->next = tempRight;
		tempLeft->next = newBlock;
		tempRight->prev = newBlock;
		return newBlock;
	}

	//If these were the only two blocks in the list, then compact them and the compacted block is the new head and tail
	else if (blockToCompact->next->next == nullptr && blockToCompact->prev == nullptr) {
		delete[] blockToCompact->next->data;
		delete[] blockToCompact->data;
		delete blockToCompact->next;
		delete blockToCompact;
		head = newBlock;
		tail = newBlock;
		return newBlock;
	}

	//If the two blocks are the rightmost blocks in the list then they become the new tail
	else if (blockToCompact->next->next == nullptr) {
		Memory::Block* tempLeft = blockToCompact->prev;

		delete[] blockToCompact->next->data;
		delete[] blockToCompact->data;
		delete blockToCompact->next;
		delete blockToCompact;

		newBlock->prev = tempLeft;
		tempLeft->next = newBlock;

		tail = newBlock;
		return newBlock;
	}

	//If the two blocks are the leftmost blocks in the list then they become the new head
	else if (blockToCompact->prev == nullptr) {
		Memory::Block* tempRight = blockToCompact->next->next;

		delete[] blockToCompact->next->data;
		delete[] blockToCompact->data;
		delete blockToCompact->next;
		delete blockToCompact;

		newBlock->next = tempRight;
		tempRight->prev = newBlock;

		head = newBlock;
		return newBlock;
	}
	return nullptr;
}

Memory::Block* Memory::FindByOffset(const unsigned int& offset) {
	Block* current = head;
	while (current->next != nullptr) {
		if (current->offset == offset) {
			return current;
		}
		current = current->next;
	}
	if (current->offset == offset) {
		return current;
	}
	else {
		return nullptr;
	}
}

//Loops through the list to find a block whose data matches the one that is being looked for
Memory::Block* Memory::FindByData(const uint64_t* dataToFind) {
	Block* current = head;
	while (current->next != nullptr) {
		if (current->data == dataToFind) {
			return current;
		}
		current = current->next;
	}
	if (current->data == dataToFind) {
		return current;
	}
	else {
		return nullptr;
	}
}

//Loops through the list and finds the offset and size of all blocks which are free
void Memory::FindFreeBlocks(std::vector<std::pair<unsigned int, unsigned int>>& v) {
	Block* current = head;
	while (current->next != nullptr) {
		if (!current->used) {
			std::pair<unsigned int, unsigned int> temp = std::make_pair(current->getOffset(), current->getSize());
			v.push_back(temp);
		}
		current = current->next;
	}
	if (!current->used) {
		std::pair<unsigned int, unsigned int> temp = std::make_pair(current->getOffset(), current->getSize());
		v.push_back(temp);
	}
}

//Loops through the list and gets the data from all blocks which are allocated
uint64_t* Memory::FindFilledBlocks() {
	std::vector <uint64_t> v;
	Block* current = head;
	while (current->next != nullptr) {
		if (current->used) {
			for (unsigned int ii = 0; ii < current->size; ii += 1) {
				v.push_back(current->data[ii]);
			}
		}
		current = current->next;
	}
	if (current->used) {
		for (unsigned int ii = 0; ii < current->size; ii += 1) {
			v.push_back(current->data[ii]);
		}
	}
	if (listData != nullptr) {
		delete[] listData;
		listData = new uint64_t[v.size()];

	}
	else {
		listData = new uint64_t[v.size()];
	}
	for (unsigned int ii = 0; ii < v.size(); ii += 1) {
		listData[ii] = v.at(ii);
	}
	return listData;
}

//Loops through the list and for each block, places a number of elements equal to the size of the block into the vector. These elements will be all 0 for a free block or all 1 for a used block
void Memory::BitRepresentation(std::vector<int>& v) {
	Block* current = head;
	while (current->next != nullptr) {
		if (current->used) {
			for (int ii = 0; ii < current->size; ii += 1) {
				v.push_back(1);
			}
		}
		else {
			for (int ii = 0; ii < current->size; ii += 1) {
				v.push_back(0);
			}
		}
		current = current->next;
	}
	if (current->used) {
		for (int ii = 0; ii < current->size; ii += 1) {
			v.push_back(1);
		}
	}
	else {
		for (int ii = 0; ii < current->size; ii += 1) {
			v.push_back(0);
		}
	}
}

//The following are all getters and modifiers for our list variables
unsigned int Memory::GetCapacity() {
	return memory_capacity;
}

void Memory::FillBlock(Block* blockToFill) {
	blockToFill->set_block_status(true);
}




