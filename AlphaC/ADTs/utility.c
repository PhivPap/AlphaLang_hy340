#include "definitions.h"
#include "v_definitions.h"

const char* typeToString[] = {
	"global variable", "local variable", "formal argument" , "user function", "library function"
};

const char* spaceToString[] = {
	"program variable", "function local", "formal_argument"
};

const char* exprtypeToString[] = {
	"var", "table", "user function", "library function", "arithm_e", "boolean expression", "assign", "newtable",  
	"constnum_e","constant boolean", "constant string", "nil"
};

const char* iopcodeToString[] = {
	"assign", "add", "sub", "mul", "div", "mod", "uminus", "and", "or", "not", "if_eq", "if_noteq", 
	"if_lessereq", "if_greatereq", "if_less", "if_greater", "call", "param", "ret", 
	"getretval", "funcstart", "funcend", "tablecreate", "tablegetelem", "tablesetelem", "jump"
};

const char* vmopcodeToString[] = {
	"assign_v", "add_v", "sub_v", "mul_v", "div_v", "mod_v", "uminus_v", "and_v", "or_v", "not_v", "if_eq_v", "if_noteq_v", 
	"if_lessereq_v", "if_greatereq_v", "if_less_v", "if_greater_v", "callfunc_v", "pusharg_v", "ret_v",
	"getretval_v", "funcenter_v", "funcend_v", "tablecreate_v", "tablegetelem_v", "tablesetelem_v", "jump_v"
};

const char* vmarg_tToString[] = {
	"label_a", "global_a", "formal_a", "local_a", "number_a", "string_a", "bool_a", "nil_a",
	"userfunc_a", "libfunc_a", "retval_a", "no_vmarg"
};

const char* boolToString[] = {
	"false" , "true"
};

void printUnionMember(Expr *expr) {
	assert(expr);

	switch(expr->type) {
		case constnum_e:	printf("%.1f\n", expr->value.numConst);	break;
		case constbool_e:	expr->value.boolConst == 0 ? printf("'false'\n") : printf("'true'\n"); break;
		case conststring_e:	printf("\"%s\"\n", expr->value.strConst); break;
		default:	printf("%s\n", expr->value.sym->name); break;
	}
}

unsigned char isTempExpr(Expr* e){
	assert(e);
	return (e->union_i == 0 && *e->value.sym->name == '_') ? 1 : 0;
}

bool CompareDoubles(void* data1, void* data2){
	return *(double*) data1 == *(double*) data2;
}

bool CompareStrings(void* data1, void* data2){
	return !strcmp((char*) data1, (char*) data2);
}

bool CompareUserfuncAddress(void* data1, void* data2){
	return ((struct userfunc*) data1)->address == ((struct userfunc*) data2)->address;
}