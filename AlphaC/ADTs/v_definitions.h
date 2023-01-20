#ifndef _V_DEFINITIONS_H_
#define _V_DEFINITIONS_H_

typedef struct vmarg vmarg;
typedef struct instruction Instruction;
typedef struct userfunc userfunc;
typedef struct incomplete_jump incomplete_jump;
typedef enum vmarg_t vmarg_t;
typedef enum vmopcode vmopcode;

enum vmopcode {
	assign_v, add_v, sub_v, mul_v, div_v, mod_v, uminus_v, and_v, or_v, not_v, \
	jeq_v, jne_v, jle_v, jge_v, jlt_v, jgt_v, call_v, pusharg_v, ret_v , get_retval_v, \
	funcenter_v, funcexit_v, newtable_v, tablegetelem_v, tablesetelem_v, jump_v, nop_v
};

enum vmarg_t {
	label_a, global_a, formal_a, local_a, number_a, string_a, bool_a, nil_a, \
	userfunc_a, libfunc_a, retval_a, no_vmarg
};

struct vmarg {
	vmarg_t type;
	uint val;
};

struct instruction {
	vmopcode op;
	vmarg result;
	vmarg arg1;
	vmarg arg2;
	uint srcLine;
};

struct userfunc {
	uint address;
	uint localSize;
	char* id;
};

struct incomplete_jump {
	uint instrNo;
	uint iaddress;
	incomplete_jump* next;
};

#endif /* _V_DEFINITIONS_H_ */


/*
	field |  used in opcode
	-----------------------
	result : return,call,param,getretval,funcstart,funcend,tablecreate

	label  : jump,
  
    result, arg1: assign

    result, arg1, arg2 : arithmetic ops , tableget, tableset
	
	label, arg1, arg2 : relational ops

	

*/
