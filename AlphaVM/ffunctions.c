#include "functions.h"
#include "libfunc.h"

static inline void (*avm_getlibraryfunc(char* id))(char*){
	/* hasd id to libfuncs table */
	uint hashValue = 0U;
	uint i = 0U;
	while(id[i]){
		hashValue = hashValue * F_HASH_MULTIPLIER + id[i];
		i++;
	}
	return executeLibFuncs[hashValue % F_ARRAY_SIZE];
}

static inline void avm_calllibfunc(char* id){
	void (*f)(char*) = avm_getlibraryfunc(id);
	assert(f);

	topsp = top;
	totalActuals = 0;
	f(id);
	execute_funcexit(NULL);
}

void execute_call(instruction *instr){
	avm_memcell* func = avm_translate_operand(instr->res_type, instr->res_val, &ax);
	assert(func);
	
	switch(func->type){

		case userfunc_m:
			avm_callsaveenvironment();
			PC = func->data.funcVal;
			assert(instructions[PC].op == 20);/* funcenter */
			break;

		case string_m:
			avm_callsaveenvironment();
			avm_calllibfunc(func->data.strVal);
			break;

		case libfunc_m:
			avm_callsaveenvironment();
			avm_calllibfunc(func->data.libfuncVal);
			break;
		
		case table_m:{
			avm_memcell call;
			call.type = string_m;
			call.data.strVal = "()";
			avm_memcell* fvalue = Atable_get_elem(func->data.tableVal, &call);
			if(!fvalue){
				avm_error("Table has no functor.\n");
			} 
			else {
				if(fvalue->type != userfunc_m){
					avm_error("Not a function at key: '()' of table.\n");
				}
				avm_assign(&stack[top], func);
				totalActuals++;
				avm_dec_top();
				avm_callsaveenvironment();
				PC = fvalue->data.funcVal;
				assert(instructions[PC].op == 20);
			}
			break;
		}

		default: {
			avm_error("call: cannot bind %s to function", memcell_typeToString[func->type]);
		}
	}
}

void execute_pusharg(instruction *instr){
	PC++;
	avm_memcell* arg = avm_translate_operand(instr->res_type, instr->res_val, &ax);
	assert(arg);
	avm_assign(&stack[top], arg);
	totalActuals++;
	avm_dec_top();
}

static inline userfunc* avm_getfuncinfo(uint address){
	for(uint i = 0; i < userfuncs_a; i++){
		if(address == userfunc_array[i].address)
			return &userfunc_array[i];
	}
	return NULL;
}

void execute_funcenter(instruction *instr){
	#ifdef __DEBUG
		avm_memcell* func = avm_translate_operand(instr->res_type, instr->res_val, &ax);
	#else 
		avm_translate_operand(instr->res_type, instr->res_val, &ax);
	#endif
	assert(func);
	assert(PC == func->data.funcVal);

	totalActuals = 0;
	userfunc* funcInfo = avm_getfuncinfo(PC);
	assert(funcInfo);
	topsp = top;
	top = top - funcInfo->localSize;
	avm_memcellclear(&retval);
	PC++;
}

void execute_funcexit(instruction *instr){
	uint oldTop = top;
	top = avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	PC = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	while(++oldTop <= top)
		avm_memcellclear(&stack[oldTop]);
}
