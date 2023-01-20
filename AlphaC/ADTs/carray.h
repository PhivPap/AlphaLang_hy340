#ifndef _C_ARRAY_H_
#define _C_ARRAY_H_ 

#include <assert.h>
#include <stdlib.h>

typedef struct ConstsArray *ConstsArray;
typedef unsigned int uint;
typedef unsigned char bool;

ConstsArray ConstsArray_new();

void ConstsArray_free(ConstsArray this);

uint ConstsArray_getLength(ConstsArray this);

uint ConstsArray_getSize(ConstsArray this);

void* ConstsArray_getData(ConstsArray this, uint index);

uint ConstsArray_getDataIndex (	ConstsArray this, void* data,
								bool (*compare)(void* data1, void* data2));

void** ConstsArray_getArray(ConstsArray this);
 
uint ConstsArray_append(ConstsArray this, void* data);

uint ConstsArray_append_optimized (	ConstsArray this, void* data,
									bool (*compare)(void* data1, void* data2));

#endif /*_C_ARRAY_H_*/