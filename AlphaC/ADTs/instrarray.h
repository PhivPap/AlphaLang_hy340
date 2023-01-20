#ifndef _INSTRARRAY_H_
#define _INSTRARRAY_H_

#include "definitions.h"
#include "v_definitions.h"
#include "quadarray.h"
#include "carray.h"

typedef struct InstrArray *InstrArray;

extern InstrArray instructions;
extern ConstsArray consts_string;
extern ConstsArray consts_number;
extern ConstsArray libfuncs_used;
extern ConstsArray userfuncs_used;

InstrArray InstrArray_new(uint startingSize);

void InstrArray_free(InstrArray this);

void InstrArray_insert_instr(InstrArray this, Instruction* instr);

void InstrArray_print(InstrArray this);

InstrArray generate_target_code(QuadArray quads);

bool InstrArray_serializer(InstrArray this, char *fileName, uint globals_num);

#endif /*_INSTRARRAY_H_*/