#include "MemoryAlgorithms.h"

//______________________________________________________________________________Memory Algorithms_______________________________________________________________________________

//Returns the smallest hole that fits the sizeInWords
int bestFit(int sizeInWords, void* list) {
	uint16_t* holeList = static_cast<uint16_t*>(list);

	//If the holeList was nullptr, return -1
	if (holeList == nullptr) {
		return -1;
	}
	else {
		//Set offset to -1 by default, the first index in the array is the number of holes in the list 
		uint16_t holeListlength = *holeList++;
		uint16_t minVal = UINT16_MAX;
		int offset = -1; 

		//Each hole provides offset info and size info so the total size to loop through is (holListLength*2)
		//We are looking at the hole size which is held in every odd index
		for (uint16_t ii = 1; ii < (holeListlength) * 2; ii += 2) {
			//If a hole has a smaller size than the current minimum size and fits the sizeInWords we're trying to allocate 
			//Then set the new offset to holeList[ii-1] (offsets stored before hole size in the list) and the minVal to the holeSize at current index
			if (holeList[ii] < minVal && sizeInWords <= holeList[ii]) {
				offset = (int)holeList[ii - 1];
				minVal = holeList[ii];
			}
		}
		//Returns offset
		return offset; 
	}
}

//Returns the largest hole that fits the sizeInWords
int worstFit(int sizeInWords, void* list) {
	uint16_t* holeList = static_cast<uint16_t*>(list);

	//If the holeList was nullptr, return -1
	if (holeList == nullptr) {
		return -1;
	}
	else {
		//Set offset to -1 by default, the first index in the array is the number of holes in the list 
		uint16_t holeListlength = *holeList++;
		uint16_t maxVal = 0;
		int offset = -1;

		//Each hole provides offset info and size info so the total size to loop through is (holListLength*2)
		//We are looking at the hole size which is held in every odd index
		for (uint16_t ii = 1; ii < (holeListlength) * 2; ii += 2) {
			//If a hole has a larger size than the current minimum size and fits the sizeInWords we're trying to allocate 
			//Then set the new offset to holeList[ii-1] (offsets stored before hole size in the list) and the maxVal to the holeSize at current index
			if (holeList[ii] > maxVal && sizeInWords <= holeList[ii]) {
				offset = (int)holeList[ii - 1];
				maxVal = holeList[ii];
			}
		}
		//Returns offset
		return offset;
	}
}