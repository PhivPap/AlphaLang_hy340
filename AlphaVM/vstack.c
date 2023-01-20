#include "vstack.h"

#define AMV_STACKENV_SIZE 4

avm_memcell stack[AVM_STACKSIZE];
avm_memcell ax,bx,cx;
avm_memcell retval;
uint top,topsp;
uint glob_mem;
uint totalActuals;

const char* memcell_typeToString[] = {
	"number", "string", "bool", "table", "userfunc", "libfunc", "nil", "undef"
}; // tha paei se kana utility me alla later ..

void avm_initstack(uint globals_a){
	for(uint i = 0; i < AVM_STACKSIZE; ++i){
		//AVM_WIPEOUT(stack[i]);
		stack[i].type = undef_m;
	}
	ax.type = undef_m;
	bx.type = undef_m;
	cx.type = undef_m;
	retval.type = undef_m;
	glob_mem = AVM_STACKSIZE -1 - globals_a;
	top = glob_mem;
}

/* A pair of type and val represents a vmarg */
avm_memcell* avm_translate_operand(unsigned char type, uint val, avm_memcell* reg){

	switch(type) {

		case global_a: return &stack[AVM_STACKSIZE -1 - val];
		case local_a: return &stack[topsp - val];
		case formal_a: return &stack[topsp + AMV_STACKENV_SIZE +1 + val];
		
		case retval_a: return &retval;

		case number_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = number_m;
			reg->data.numVal = number_array[val];
			return reg;
		
		case string_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = string_m;
			reg->data.strVal = strdup(string_array[val]);
			return reg;

		case bool_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = bool_m; 
			reg->data.boolVal = val;
			return reg;

		case nil_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = nil_m;
			return reg;

		case userfunc_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = userfunc_m;
			reg->data.funcVal = userfunc_array[val].address;
			return reg;

		case libfunc_a:
			assert(reg);
			avm_memcellclear(reg);
			reg->type = libfunc_m;
			reg->data.libfuncVal = libfunc_array[val]; // ??
			return reg;

		case no_vmarg:
			avm_memcellclear(reg);
			return reg;
		default: assert(0);
	}
	return NULL;
}


void avm_assign(avm_memcell* lv, avm_memcell* rv){
	//checks
	if(lv == rv)
		return;
	
	if(lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal)
		return;

	// if(rv->type == undef_m)
		//printf("jesus christ\n");
		/* warning ? */
	
	avm_memcellclear(lv);
	memcpy(lv, rv, sizeof(avm_memcell));
	//lv->type = rv->type;
	//lv->data = rv->data;
	if(lv->type == string_m)
		lv->data.strVal = strdup(rv->data.strVal);
	if(lv->type == table_m) /* mark root ? */
		Atable_increase_ref(lv->data.tableVal);
}

void avm_memcellclear(avm_memcell* memcell){
	if(memcell->type == string_m){
		assert(memcell->data.strVal);
		free(memcell->data.strVal);
	}
	else if(memcell->type == table_m){
		assert(memcell->data.tableVal);
		Atable_decrease_ref(memcell->data.tableVal);
	}
	memcell->type = undef_m;
}

void avm_dec_top(){
	if(top <= 0)
		avm_error("Stack overflow.\n");
	else
		top--;
}

static void avm_push_envvalue(uint val){
	stack[top].type = number_m;
	stack[top].data.numVal = val;
	avm_dec_top();
}

void avm_callsaveenvironment(){
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(PC + 1);
	avm_push_envvalue(top + totalActuals + 2);
	avm_push_envvalue(topsp);
}

uint avm_get_envvalue(uint i){
	assert(stack[i].type == number_m);
	uint val = (uint) stack[i].data.numVal;
	assert(stack[i].data.numVal == (double)val);
	return val;
}

uint avm_totalactuals(){
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

avm_memcell* avm_getactual(uint i){
	assert(i < avm_totalactuals());
	return &stack[topsp + AMV_STACKENV_SIZE + 1 + i];
}

void avm_error(char* fmt, ...){
	
	printf("\nAVM ERROR: (line %u)\n", currLine);

	va_list args;
    va_start(args, fmt);
 
    while (*fmt != '\0') {

        if (*fmt == '%') {
        	fmt++;
      
        	if(*fmt == 's'){
        		printf("%s", va_arg(args, char*));
        	}
        	else if(*fmt == 'd'){
	           	printf("%d", va_arg(args, int));
        	}
        	else if(*fmt == 'u'){
                printf("%u", va_arg(args, uint));}
        	else if(*fmt == '\n'){
        		printf("\n");
        	}
        	else {
        		printf("%%");
        		printf("%c", *fmt);
        	}
        }
        else{
        	printf("%c", *fmt);
        }
        ++fmt;
    }
    va_end(args);
    exit(1);
}

static inline bool isInt(double num){
	return num == (int)num;
}

char* avm_tostring(avm_memcell* m){
	char *buff;
	switch(m->type){
		case number_m:{
			buff = malloc(25);
			if(isInt(m->data.numVal))
				sprintf(buff, "%.0lf", m->data.numVal);
			else
				sprintf(buff, "%.3lf", m->data.numVal);
			return buff;
		}
		case string_m:{
			buff = strdup(m->data.strVal);
			return buff;
		}
		case bool_m:{
			buff = malloc(6);
			m->data.boolVal == 0 ? strcpy(buff,"false") : strcpy(buff,"true");
			return buff;
		}
		case table_m:{
			return Atable_getTable(m->data.tableVal);
		}
		case userfunc_m:{
			buff = malloc(20);
			sprintf(buff, "userfunc %u", m->data.funcVal);
			return buff;
		}
		case libfunc_m:{
			buff = malloc(27);
			sprintf(buff, "libfunc %s", m->data.libfuncVal);
			return buff;
		}
		case nil_m:{
			buff = malloc(4);
			strcpy(buff, "nil");
			return buff;
		}
		case undef_m:{
			avm_error("Cannot print undefined.\n");
		}

		default:assert(0);
	}
	return NULL;
}


//ah shit, here wego agane