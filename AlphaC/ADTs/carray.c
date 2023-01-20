#include "carray.h"
#include "stdio.h"
#define DEFAULT_CARRAY_SIZE 128

struct ConstsArray {
	void** data;
	uint num_data;
	uint size;
};

ConstsArray ConstsArray_new(){
	ConstsArray this = malloc(sizeof(struct ConstsArray));
	this->num_data = 0;
	this->size = DEFAULT_CARRAY_SIZE;
	this->data = calloc(this->size, sizeof(void*));
	return this;
}

void ConstsArray_free(ConstsArray this){
	free(this->data);
	free(this);
}

static void ConstsArray_expand(ConstsArray this){
	assert(this);
	assert(this->num_data == this->size);
	this->size += DEFAULT_CARRAY_SIZE;
	this->data = realloc(this->data, sizeof(void*) * this->size);
}

uint ConstsArray_getLength(ConstsArray this){
	assert(this);
	assert(this->num_data >= 0 && this->num_data <= this->size);
	return this->num_data;
}

uint ConstsArray_getSize(ConstsArray this){
	assert(this);
	assert(this->size % DEFAULT_CARRAY_SIZE == 0);
	return this->size;
}

void* ConstsArray_getData(ConstsArray this, uint index){
	assert(this);
	assert(index >= 0 && index < this->size);
	return this->data[index];
}

uint ConstsArray_getDataIndex (	ConstsArray this, void* data, 
								bool (*compare)(void* data1, void* data2)) {
	assert(this);
	for(uint i = 0; i < this->num_data; i++)
		if( (*compare)(this->data[i], data) == 0)
			return i;
}

void** ConstsArray_getArray(ConstsArray this){
	return this->data;
}

uint ConstsArray_append(ConstsArray this, void* data){
	assert(this);
	uint index = this->num_data;
	if(index == this->size)
		ConstsArray_expand(this);
	this->data[this->num_data++] = data;
	return index;
}

uint ConstsArray_append_optimized (	ConstsArray this, void* data,
									bool (*compare)(void* data1, void* data2)) {
	assert(this);
	uint index = this->num_data;
	for(uint i = 0; i < index; i++)
		if( (*compare)(this->data[i], data) == 1)
			return i;
	/* else append void* data */
	if(index == this->size)
		ConstsArray_expand(this);
	this->data[this->num_data++] = data;
	return index;
}