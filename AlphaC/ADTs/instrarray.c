#include "instrarray.h"

#define INSTRUCTION_EXPAND_SIZE 1024

#define emit(args...) InstrArray_insert_instr(instructions, ##args)

#define PRINT_INSTR_SKIP_PART printf("                    ");

InstrArray instructions 	= NULL;
ConstsArray consts_string 	= NULL;
ConstsArray consts_number 	= NULL;
ConstsArray libfuncs_used 	= NULL;
ConstsArray userfuncs_used 	= NULL;
Quad * arrayOfQuads			= NULL;
incomplete_jump *jhead 		= NULL;
uint currProcessedQuad;
uint Quads_Length;
bool funcEnter;

void jhead_add(uint instrNo, uint iaddress){
	incomplete_jump *this = malloc(sizeof(struct incomplete_jump));
	this->instrNo = instrNo;
	this->iaddress = iaddress;
	this->next = jhead;
	jhead = this;
}

struct InstrArray {
	uint size;
	uint currLen;
	Instruction* instructions;
};

InstrArray InstrArray_new(uint startingSize){
	InstrArray this = malloc(sizeof(struct InstrArray));
	this->instructions = calloc(startingSize, sizeof(struct instruction));
	this->size = startingSize;
	this->currLen = 0;
	return this;
}

void InstrArray_free(InstrArray this){
	assert(this);
	free(this->instructions);
	free(this);
}

static uint InstrArray_nextInstr(InstrArray this){
	assert(this);
	return this->currLen;
}

static void InstrArray_patch_jump(InstrArray this, uint instrNo, uint patch){
	assert(this);
	this->instructions[instrNo].result.val = patch;
}

static void InstrArray_patch_incomplete_jumps(InstrArray this){
	assert(this);
	incomplete_jump *prev;
	while(jhead){
		prev = jhead;
		if(jhead->iaddress >= Quads_Length)
			InstrArray_patch_jump(this, jhead->instrNo, InstrArray_nextInstr(this));
		else
			InstrArray_patch_jump(this, jhead->instrNo, arrayOfQuads[jhead->iaddress].taddress);
		jhead = jhead->next;
		free(prev);
	}
}

static void InstrArray_expand(InstrArray this){
	assert(this);
	assert(this->size == this->currLen);
	this->size += INSTRUCTION_EXPAND_SIZE;
	this->instructions = realloc(this->instructions, sizeof(struct instruction) * this->size);
}

void InstrArray_insert_instr(InstrArray this, Instruction* instr){
	assert(this);
	if(this->currLen == this->size)
		InstrArray_expand(this);
	Instruction* p = &(this->instructions[this->currLen++]);

	p->op = instr->op;
	p->arg1 = instr->arg1;
	p->arg2 = instr->arg2;
	p->result = instr->result;
	p->srcLine = instr->srcLine;
}

void make_operand(Expr* e, vmarg* arg){
	if(!e){
		arg->type = no_vmarg;
		arg->val = 0;
		return;
	}

	switch(e->type) {
		case var_e:
		case tableitem_e:
		case arithm_e:
		case bool_e:
		case newtable_e:
			assert(e->union_i == 0); /* == sym */
			
			arg->val = e->value.sym->value.varVal->offset;

			switch(e->value.sym->value.varVal->space) {
				case program_var:	arg->type = global_a; break;
				case function_local: arg->type = local_a; break;
				case formal_argument: arg->type = formal_a; break;
			}
			break;
		/* Constants */
		case constbool_e:
			assert(e->union_i == 3); /* == boolConst */
			arg->val = e->value.boolConst;
			arg->type = bool_a;
			break;
		case conststring_e:
			assert(e->union_i == 2); /* == strConst */
			arg->val = ConstsArray_append_optimized(consts_string, strdup(e->value.strConst), CompareStrings);
			arg->type = string_a;
			break;
		case constnum_e:
			assert(e->union_i == 1); /* == numConst */
			double * tmp = malloc(sizeof(double));
			*tmp = e->value.numConst;
			arg->val = ConstsArray_append_optimized(consts_number, tmp, CompareDoubles);
			arg->type = number_a;
			break;
		case nil_e:
			assert(e->union_i == 4); /* == nil */
			arg->type = nil_a;
			arg->val = 0;
			break;

		/* Functions */
		case programfunc_e:
			assert(e->union_i == 0);
			struct userfunc *uf = malloc(sizeof(struct userfunc));
			uf->id = strdup(e->value.sym->name);
			if(funcEnter){
				uf->address = InstrArray_nextInstr(instructions);
				e->value.sym->value.funcVal->address = uf->address;
			}
			else{
				uf->address = e->value.sym->value.funcVal->address;
			}
			uf->localSize = e->value.sym->value.funcVal->local_size;
			arg->type = userfunc_a;
			arg->val = ConstsArray_append_optimized(userfuncs_used, uf, CompareUserfuncAddress);
			break;
		case libfunc_e:
			assert(e->union_i == 0);
			arg->type = libfunc_a;
			arg->val = ConstsArray_append_optimized(libfuncs_used, strdup(e->value.sym->name), CompareStrings);
			break;

		default: assert(0);
	}
}

void generate(Quad* quad){
	quad->qaddress = InstrArray_nextInstr(instructions);
	Instruction t;
	t.op = quad->op;
	make_operand(quad->arg1, &t.arg1);
	make_operand(quad->arg2, &t.arg2);
	make_operand(quad->result,  &t.result);
	t.srcLine = quad->line;
	quad->taddress = InstrArray_nextInstr(instructions);
	emit(&t);
}

void generate_relational(Quad* quad){
	Instruction t;
	t.op = quad->op;
	make_operand(quad->arg1, &t.arg1);
	make_operand(quad->arg2, &t.arg2);
	t.srcLine = quad->line;

	t.result.type = label_a;
	
	#ifdef __OPT
		while(quad->label < Quads_Length && arrayOfQuads[quad->label].ignore == 1)
			quad->label++;
	#endif

	if (quad->label < currProcessedQuad)
		t.result.val = arrayOfQuads[quad->label].taddress;
	else
		jhead_add(InstrArray_nextInstr(instructions), quad->label);
	
	quad->taddress = InstrArray_nextInstr(instructions); 
	
	emit(&t);
}

static void generate_ASSIGN(Quad* quad){ generate(quad);}
static void generate_ADD(Quad* quad){ generate(quad);}
static void generate_SUB(Quad* quad){ generate(quad);}
static void generate_MUL(Quad* quad){ generate(quad);}
static void generate_DIV(Quad* quad){ generate(quad);}
static void generate_MOD(Quad* quad){ generate(quad);}
static void generate_UMINUS(Quad* quad){ assert(0); }
static void generate_AND(Quad* quad){ assert(0);}
static void generate_OR(Quad* quad){ assert(0);}
static void generate_NOT(Quad* quad){ assert(0);}
static void generate_IFEQ(Quad* quad){ generate_relational(quad);}
static void generate_IFNOTEQ(Quad* quad){ generate_relational(quad);}
static void generate_IFLESSEREQ(Quad* quad){ generate_relational(quad);}
static void generate_IFGREATEREQ(Quad* quad){ generate_relational(quad);}
static void generate_IFLESS(Quad* quad){ generate_relational(quad);}
static void generate_IFGREATER(Quad* quad){ generate_relational(quad);}
static void generate_CALL(Quad* quad){ generate(quad);}
static void generate_PARAM(Quad* quad){generate(quad);}
static void generate_RET(Quad* quad){ 
	Instruction t;
	t.op = assign_v;
	t.result.type = retval_a;
	t.result.val = 0;
	t.arg2.type = no_vmarg;
	t.arg2.val = 0;
	make_operand(quad->result, &t.arg1);
	t.srcLine = quad->line;
	quad->taddress = InstrArray_nextInstr(instructions);
	emit(&t);
}
static void generate_GETRETVAL(Quad* quad){ 
	Instruction t;
	t.op = assign_v;
	make_operand(quad->result, &t.result);
	t.arg1.type = retval_a;
	t.arg1.val = 0;
	t.arg2.type = no_vmarg;
	t.arg2.val = 0;
	t.srcLine = quad->line;
	quad->taddress = InstrArray_nextInstr(instructions);
	emit(&t);
}
static void generate_FUNCSTART(Quad* quad){ funcEnter = 1; generate(quad); funcEnter = 0;}
static void generate_FUNCEND(Quad* quad){ generate(quad);}
static void generate_TABLECREATE(Quad* quad){ generate(quad);}
static void generate_TABLEGETELEM(Quad* quad){ generate(quad);}
static void generate_TABLESETELEM(Quad* quad){ generate(quad);}
static void generate_JUMP(Quad* quad){ generate_relational(quad);}
static void generate_NOP(Quad* quad){ Instruction t; t.op = nop_v; emit(&t); }

void (*generators[])(Quad *) = {

generate_ASSIGN,
generate_ADD,
generate_SUB,
generate_MUL,
generate_DIV,
generate_MOD,
generate_UMINUS,
generate_AND,
generate_OR,
generate_NOT,
generate_IFEQ,
generate_IFNOTEQ,
generate_IFLESSEREQ,
generate_IFGREATEREQ,
generate_IFLESS,
generate_IFGREATER,
generate_CALL,
generate_PARAM,
generate_RET,
generate_GETRETVAL,
generate_FUNCSTART,
generate_FUNCEND,
generate_TABLECREATE,
generate_TABLEGETELEM,
generate_TABLESETELEM,
generate_JUMP,
generate_NOP

};

static void printVmarg_t(vmarg arg){
	
	printf("%02u(%s)", arg.type, vmarg_tToString[arg.type]);
	
	switch(arg.type){
		case label_a:
			printf(", %-10u", arg.val);
			break;
		
		case global_a:
		case formal_a:
		case local_a:
			printf(", %-10u", arg.val);
			break;
		
		/* doubles ConstsArray */
		case number_a:
			printf(", %u:%-10g", arg.val, *(double*)ConstsArray_getData(consts_number, arg.val));
			break;
		/* strings ConstsArray */
		case string_a:
			printf(", %u:%-10s", arg.val, (char*)ConstsArray_getData(consts_string, arg.val));
			break;
		case bool_a:
			printf(", %-10s", boolToString[arg.val]);
			break;
		case nil_a:
			break;
		/* userfuncs_used ConstsArray */
		case userfunc_a: {
			userfunc* ufp = (struct userfunc*)ConstsArray_getData(userfuncs_used, arg.val);
			printf(", %u:%s %u %u", arg.val, ufp->id, ufp->address, ufp->localSize );
			break;
		}
		/* libfuncs_used ConstsArray */
		case libfunc_a:
			printf(", %u:%s", arg.val, (char*)ConstsArray_getData(libfuncs_used, arg.val));
			break;
		case retval_a:
		/* nothing to print */
		printf("            ");
			break;
		case no_vmarg:
			assert(arg.val == 0);
			break;
		default: assert(0);
	}
}

/*enum vmarg_t {
	label_a, global_a, formal_a, local_a, number_a, string_a, bool_a, nil_a, \
	userfunc_a, libfunc_a, retval_a, no_vmarg
};*/

void InstrArray_print(InstrArray this){
	if(this == NULL){
		return;
	}
	Instruction* instr = this->instructions;
	uint currInstr = this->currLen;

	for(uint i = 0; i < currInstr; i++){
		printf("I %-6d  %-14s  ", i, vmopcodeToString[instr[i].op]);
		switch(instr[i].op){
			case assign_v:
				printf("RES: "); //part1
				printVmarg_t(instr[i].result);
				printf("   ");
				printf("ARG1: "); //part2
				printVmarg_t(instr[i].arg1);
				printf("  ");
				PRINT_INSTR_SKIP_PART
				break;
			case sub_v:
			case mul_v:
			case div_v:
			case mod_v:
			case add_v:
				printf("RES: "); //part1
				printVmarg_t(instr[i].result);
				printf("   ");
				printf("ARG1: "); //part2
				printVmarg_t(instr[i].arg1);
				printf("  ");
				printf("ARG2: "); //part3
				printVmarg_t(instr[i].arg2);
				printf("  ");
				break;
			case uminus_v:
			case and_v:
			case or_v:
			case not_v: assert(0);
			case jeq_v:
			case jne_v:
			case jle_v:
			case jge_v:
			case jlt_v:
			case jgt_v:
				printf("JUMPTO: ");
				printVmarg_t(instr[i].result);
				printf("   ");
				printf("ARG1: "); //part2
				printVmarg_t(instr[i].arg1);
				printf("  ");
				printf("ARG2: "); //part3
				printVmarg_t(instr[i].arg2);
				printf("  ");
				break;
				break;
			case pusharg_v:
			case call_v:
			case funcenter_v:
			case funcexit_v:
				printf("RES: ");
				printVmarg_t(instr[i].result);
				printf("   ");
				PRINT_INSTR_SKIP_PART
				PRINT_INSTR_SKIP_PART
			break;
			case newtable_v:
				printf("RES: "); //part1
				printVmarg_t(instr[i].result);
				printf("   ");
				PRINT_INSTR_SKIP_PART
				PRINT_INSTR_SKIP_PART
				break;
			case tablegetelem_v:
				printf("GET: "); //part1
				printVmarg_t(instr[i].result);
				printf("   ");
				printf("ARG1: "); //part2
				printVmarg_t(instr[i].arg1);
				printf("  ");
				printf("ARG2: "); //part3
				printVmarg_t(instr[i].arg2);
				printf("  ");
				break;
			case tablesetelem_v:
				printf("SET: "); //part1
				printVmarg_t(instr[i].result);
				printf("   ");
				printf("ARG1: "); //part2
				printVmarg_t(instr[i].arg1);
				printf("  ");
				printf("ARG2: "); //part3
				printVmarg_t(instr[i].arg2);
				printf("  ");
				break;
			case jump_v:
				printf("JUMPTO: ");
				printVmarg_t(instr[i].result);
				printf("  ");
				PRINT_INSTR_SKIP_PART
				PRINT_INSTR_SKIP_PART
				break;
			case nop_v:
				break;
			case ret_v:
			case get_retval_v:
				assert(0);
			default: 
				assert(0);
		}
		printf("\n");
	}
	printf("\n\n=======================================\n\n");

}

static void initiliazeConstsArray(){
	consts_string  = ConstsArray_new();
	consts_number  = ConstsArray_new();
	libfuncs_used  = ConstsArray_new();
	userfuncs_used = ConstsArray_new();
}

InstrArray generate_target_code(QuadArray quads){
	assert(quads);
	initiliazeConstsArray();
	instructions = InstrArray_new(QuadArray_getSize(quads));
	arrayOfQuads = QuadArray_getQuads(quads);
	Quads_Length = QuadArray_nextQuad(quads);
	for (currProcessedQuad = 0; currProcessedQuad < Quads_Length ; ++currProcessedQuad){
		#ifdef __OPT
			if(arrayOfQuads[currProcessedQuad].ignore != 1)
				generators[arrayOfQuads[currProcessedQuad].op](&arrayOfQuads[currProcessedQuad]);
		#else
			generators[arrayOfQuads[currProcessedQuad].op](&arrayOfQuads[currProcessedQuad]);
		#endif
	}
	InstrArray_patch_incomplete_jumps(instructions);
	return instructions;
}

static inline void freeCArrays(){
	uint amountOfStrings = ConstsArray_getLength(consts_string);
	uint amountOfDoubles = ConstsArray_getLength(consts_number);
	uint amountOfUserFuncs = ConstsArray_getLength(userfuncs_used);
	uint amountOfLibFuncs = ConstsArray_getLength(libfuncs_used);
	
	struct userfunc** ufuncs = (struct userfunc**)ConstsArray_getArray(userfuncs_used);
	for(uint i=0; i<amountOfUserFuncs; i++){
		free(ufuncs[i]->id);
		free(ufuncs[i]);
	}
	ConstsArray_free(userfuncs_used);

	char** cstrings = (char**)ConstsArray_getArray(consts_string);
	for(uint i=0; i<amountOfStrings; i++){
		free(cstrings[i]);
	}
	ConstsArray_free(consts_string);

	char** lifuncs = (char**)ConstsArray_getArray(libfuncs_used);
	for(uint i=0; i<amountOfLibFuncs; i++){
		free(lifuncs[i]);
	}
	ConstsArray_free(libfuncs_used);

	ConstsArray_free(consts_number);
}

bool InstrArray_serializer(InstrArray this, char *fileName, uint globals_num){
	if(this == NULL){
		//printf("No instructions to serialize.\n");
		return 1;
	}
	FILE *binFile = fopen(fileName, "wb");
	if(!binFile){
		perror("ERROR");
		return 1;
	}
	uint magicNumber = 340200501; //really?
	uint amountOfStrings = ConstsArray_getLength(consts_string);
	uint amountOfDoubles = ConstsArray_getLength(consts_number);
	uint amountOfUserFuncs = ConstsArray_getLength(userfuncs_used);
	uint amountOfLibFuncs = ConstsArray_getLength(libfuncs_used);
	fwrite(&magicNumber, sizeof(uint), 1, binFile);
	// STRING ARRAY =================================
	fwrite(&amountOfStrings, sizeof(uint), 1, binFile);
	char* string;
	uint stringSize;
	for(int i = 0; i < amountOfStrings; i++){
		string = (char*)ConstsArray_getData(consts_string, i);
		stringSize = strlen(string) + 1;
		fwrite(&stringSize, sizeof(uint), 1, binFile);
		fwrite(string, stringSize, 1, binFile);
	}
	// NUMBER ARRAY =================================
	fwrite(&amountOfDoubles, sizeof(uint), 1, binFile);
	
	
	for(uint i = 0; i < amountOfDoubles; i++)
		fwrite(ConstsArray_getData(consts_number,i), sizeof(double), 1, binFile);

	
	// USERFUNC ARRAY ===============================
	fwrite(&amountOfUserFuncs, sizeof(uint), 1, binFile);
	userfunc *ptr;
	for(int i = 0; i < amountOfUserFuncs; i++){
		ptr = (userfunc *)ConstsArray_getData(userfuncs_used, i);
		fwrite(&(ptr->address), sizeof(uint), 1, binFile);
		fwrite(&(ptr->localSize), sizeof(uint), 1, binFile);
		string = ptr->id;
		stringSize = strlen(string) + 1;
		fwrite(&stringSize, sizeof(uint), 1, binFile);
		fwrite(string, stringSize, 1, binFile);
	}
	// LIBFUNC ARRAY ================================
	fwrite(&amountOfLibFuncs, sizeof(uint), 1, binFile);
	for(int i = 0; i < amountOfLibFuncs; i++){
		string = (char*)ConstsArray_getData(libfuncs_used, i);
		stringSize = strlen(string) + 1;
		fwrite(&stringSize, sizeof(uint), 1, binFile);
		fwrite(string, stringSize, 1, binFile);
	}

	// INSTRUCTIONS =================================
	uint instructionsLen = InstrArray_nextInstr(this);
	fwrite(&instructionsLen, sizeof(uint), 1, binFile);
	unsigned char binOpcode;
	unsigned char binType;
	
	Instruction* arrayInstructions = this->instructions;
	
	for(int i = 0; i < instructionsLen; i++){
		
		binOpcode = (unsigned char)arrayInstructions[i].op;
		fwrite(&binOpcode, sizeof(unsigned char), 1, binFile);
		
		binType = (unsigned char)arrayInstructions[i].result.type;
		fwrite(&binType, sizeof(unsigned char), 1, binFile);

		binType = (unsigned char)arrayInstructions[i].arg1.type;
		fwrite(&binType, sizeof(unsigned char), 1, binFile);

		binType = (unsigned char)arrayInstructions[i].arg2.type;
		fwrite(&binType, sizeof(unsigned char), 1, binFile);

		
		fwrite(&arrayInstructions[i].result.val, sizeof(uint), 1, binFile);
		fwrite(&arrayInstructions[i].arg1.val, sizeof(uint), 1, binFile);
		fwrite(&arrayInstructions[i].arg2.val, sizeof(uint), 1, binFile);

		uint srcLine = arrayInstructions[i].srcLine;
		fwrite(&srcLine, sizeof(uint), 1, binFile);	
	}

	fwrite(&globals_num, sizeof(uint), 1, binFile);
	fclose(binFile);
	freeCArrays();
	return 0;
}