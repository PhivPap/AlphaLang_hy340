#include "libfunc.h"
#include <math.h>
#define PI 3.141592654
#define MAX_DBL_DIGITS 16
// Change only after running program to find new array order.
// Also F_HASH_MULTIPLIER, ARRAY_SIZE in header.
void (*executeLibFuncs[])(char*) = {
	invalid_libfunc, libfunc_sqrt, libfunc_input, libfunc_argument, libfunc_print, \
	libfunc_objecttotalmembers, libfunc_sin, libfunc_strtonum, libfunc_objectcopy, \
	libfunc_objectmemberkeys, libfunc_typeof, libfunc_cos, libfunc_totalarguments
};


static inline void assertFuncID(const char* libfuncID, char* ID){
	if(strcmp(libfuncID, ID) != 0){
		avm_error("Cannot call %s, not a function.\n", ID);
	}
}

void invalid_libfunc(char* id){
	avm_error("Cannot call %s, not a function.\n", id);
}

void libfunc_print(char* id){
	assertFuncID("print", id);
    uint n = avm_totalactuals();
    for(uint i = 0; i < n; i++){
        char* s = avm_tostring(avm_getactual(i));
        fputs(s,stdout);
        free(s);
    }
    avm_memcellclear(&retval);
    retval.type = number_m;
    retval.data.numVal = n;
}

static inline bool isDigit(char c){
	if(c >= '0' && c <= '9')
		return 1;
	return 0;
}

void libfunc_input(char* id){
	assertFuncID("input", id);
	avm_memcellclear(&retval);
	char *line = malloc(50); // default string size 50. getline reallocs.
	size_t bufferSize = 50;
	size_t actualLength = getline(&line, &bufferSize, stdin);
	line[--actualLength] = '\0';
	//printf("actualLength %d\n", actualLength);
	if(actualLength == 0){
		retval.type = nil_m;
		free(line);
		//printf("input() returns nil\n");
		return;
	}
	if(strcmp(line, "nil") == 0){
		retval.type = nil_m;
		free(line);
		//printf("input() returns nil\n");
		return;
	}
	// cant be nil:
	if(strcmp(line, "true") == 0){
		retval.type = bool_m;
		retval.data.boolVal = 1;
		free(line);
		//printf("input() returns bool with value: true\n");
		return;
	}
	if(strcmp(line, "false") == 0){
		retval.type = bool_m;
		retval.data.boolVal = 0;
		free(line);
		//printf("input() returns bool with value: false\n");
		return;
	}
	// cant be nil or bool:

	uint index = 0;
	if(line[index] == '-' || line[index] == '+'){
		index++;
	}
	if(actualLength - index <= MAX_DBL_DIGITS){
		uint afterDotIndex;
		while(isDigit(line[index]))
			index++;
		if(index == actualLength){
			retval.type = number_m;
			sscanf(line, "%lf", &retval.data.numVal);
			free(line);
			//printf("input() returns number with value: %lf\n", retval.data.numVal);
			return;
		}
		if(line[index] == '.'){
			afterDotIndex = ++index;
			while(isDigit(line[index]))
				index++;
			if(index == actualLength && index != afterDotIndex){
				retval.type = number_m;
				sscanf(line, "%lf", &retval.data.numVal);
				free(line);
				//printf("input() returns number with value: %lf\n", retval.data.numVal);
				return;
			}
		}
	}
	// cant be nil or bool or num:
	if(actualLength != 1){
		if(line[0] == '"' && line[actualLength - 1] == '"'){
			line[actualLength - 1] = '\0';
			char *newStr = malloc(actualLength - 1);
			strcpy(newStr, &line[1]);
			free(line);
			line = newStr;
		}
	}
	retval.type = string_m;
	retval.data.strVal = line;
	//printf("input() returns string with value: %s\n", retval.data.strVal);
	return;
}

void libfunc_objectmemberkeys(char* id){
	assertFuncID("objectmemberkeys", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'objectmemberkeys' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != table_m)
		avm_error("library function 'objectmemberkeys' expected table argument but got %s.\n", memcell_typeToString[arg->type]);
	avm_memcellclear(&retval);
	retval.type = table_m;
	retval.data.tableVal = Atable_copy_memberkeys(arg->data.tableVal);
	Atable_increase_ref(retval.data.tableVal);
}

void libfunc_objecttotalmembers(char* id){
	assertFuncID("objecttotalmembers", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'objecttotalmembers' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != table_m)
		avm_error("library function 'objecttotalmembers' expected table argument but got %s.\n", memcell_typeToString[arg->type]);
	avm_memcellclear(&retval);
	retval.type = number_m;
	retval.data.numVal = Atable_get_currSize(arg->data.tableVal);
}

void libfunc_objectcopy(char* id){
	assertFuncID("objectcopy", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'objectcopy' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != table_m)
		avm_error("library function 'objectcopy' expected table argument but got %s.\n", memcell_typeToString[arg->type]);
	avm_memcellclear(&retval);
	retval.type = table_m;
	retval.data.tableVal = Atable_copyObj(arg->data.tableVal);
	Atable_increase_ref(retval.data.tableVal);
}

void libfunc_totalarguments(char* id){
	assertFuncID("totalarguments", id);
	uint p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	avm_memcellclear(&retval);
	if(!p_topsp){
		printf("AVM WARNING: (line %u)\n>>library function 'totalarguments' called outside function.\n", currLine);
		retval.type = nil_m;
		return;
	}
	retval.type = number_m;
	retval.data.numVal = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
}

static inline bool isInt(double num){
	if (num - (int)num == 0)
	     return 1;
	return 0;
}

void libfunc_argument(char* id){
	assertFuncID("argument", id);
	uint p_topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	if(!p_topsp){
		avm_memcellclear(&retval);
		printf("AVM WARNING: (line %u)\n>>library function 'argument' called outside function.\n", currLine);
		retval.type = nil_m;
		return;
	}
		
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'argument' expected 1 argument but got %u.\n", n);
	uint total_argc = avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
	avm_memcell* arg = avm_getactual(0); 
	if(arg->type != number_m)
		avm_error("library function 'argument' expected number argument but got %s.\n", memcell_typeToString[arg->type]);
	if(arg->data.numVal < 0 || arg->data.numVal >= total_argc || !isInt(arg->data.numVal))
		avm_error("library function 'argument': there is no argument '%s'.\n", avm_tostring(arg));
	
	avm_assign(&retval, &stack[p_topsp + AVM_NUMACTUALS_OFFSET + 1 + (int)arg->data.numVal]);	
}


void libfunc_typeof(char* id){
	assertFuncID("typeof", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'typeof' expected 1 argument but got %u\n", n);
	avm_memcellclear(&retval);
	retval.type = string_m;
	retval.data.strVal = strdup(memcell_typeToString[avm_getactual(0)->type]);
}

void libfunc_strtonum(char* id){
	assertFuncID("strtonum", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'strtonum' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != string_m)
		avm_error("library function 'strtonum' expected string argument but got %s.\n", memcell_typeToString[arg->type]);
	
	avm_memcellclear(&retval);
	
	char* str = arg->data.strVal;
	uint index = 0;
	uint afterDotIndex;
	uint actualLength = strlen(str);

	if(str[index] == '-' || str[index] == '+'){
		index++;
	}
	if(actualLength - index > MAX_DBL_DIGITS){
		retval.type = nil_m;
		return;
	}
	while(isDigit(str[index]))
		index++;
	if(index == actualLength){
		retval.type = number_m;
		sscanf(str, "%lf", &retval.data.numVal);
		return;
	}
	if(str[index] == '.'){
		afterDotIndex = ++index;
		while(isDigit(str[index]))
			index++;
		if(index == actualLength && index != afterDotIndex){
			retval.type = number_m;
			sscanf(str, "%lf", &retval.data.numVal);
			//printf("input() returns number with value: %lf\n", retval.data.numVal);
			return;
		}
	}
	retval.type = nil_m;
}

void libfunc_sqrt(char* id){
	assertFuncID("sqrt", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'sqrt' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != number_m)
		avm_error("library function 'sqrt' expected number argument but got %s.\n", memcell_typeToString[arg->type]);
	
	avm_memcellclear(&retval);

	if(arg->data.numVal < 0){
		retval.type = nil_m;
		return;
	}
	retval.type = number_m;
	retval.data.numVal = sqrt(arg->data.numVal);
}

void libfunc_cos(char* id){
	assertFuncID("cos", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'cos' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != number_m)
		avm_error("library function 'cos' expected number argument but got %s.\n", memcell_typeToString[arg->type]);
	
	avm_memcellclear(&retval);

	retval.type = number_m;
	retval.data.numVal = cos(arg->data.numVal * PI / 180.0);
}

void libfunc_sin(char* id){
	assertFuncID("sin", id);
	uint n = avm_totalactuals();
	if(n != 1)
		avm_error("library function 'sin' expected 1 argument but got %u.\n", n);
	avm_memcell* arg = avm_getactual(0);
	if(arg->type != number_m)
		avm_error("library function 'sin' expected number argument but got %s.\n", memcell_typeToString[arg->type]);
	
	avm_memcellclear(&retval);

	retval.type = number_m;
	retval.data.numVal = sin(arg->data.numVal * PI / 180.0);
}
