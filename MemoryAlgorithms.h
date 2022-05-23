#pragma once
#include "MemoryManager.h"

//Declares global memory algorithm functions
#ifndef Memory_Algorithm_Header
#define Memory_Alorithm_Header
int bestFit(int sizeInWords, void* list);
int worstFit(int sizeInWords, void* list);
#endif