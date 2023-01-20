#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef __DEBUG
	#include <assert.h>
#else
	#define assert(...)
#endif

typedef unsigned int uint;
typedef unsigned char bool;

typedef struct SymbolTableEntry SymbolTableEntry;
typedef struct arg Arg_node;
typedef struct Function Function;
typedef struct Variable Variable;
typedef struct Expr Expr;
typedef struct Quad Quad;
typedef enum iopcode iopcode;
typedef enum Expr_type Expr_type;
typedef enum scopespace_t scopespace_t;

extern const char *typeToString[];
extern const char *spaceToString[];
extern const char *exprtypeToString[];
extern const char *iopcodeToString[];
extern const char *vmopcodeToString[];
extern const char *vmarg_tToString[];
extern const char* boolToString[];
extern bool CompareDoubles(void* data1, void* data2);
extern bool CompareStrings(void* data1, void* data2);
extern bool CompareUserfuncAddress(void* data1, void* data2);

void printUnionMember(Expr* expr);
bool isTempExpr(Expr* e);


enum SymbolType { GLOBAL, _LOCAL, FORMAL, USERFUNC, LIBFUNC};

enum iopcode { 
	_assign, _add, _sub, _mul, _div, _mod, _uminus, _and, _or, _not, _if_eq, _if_noteq, \
	_if_lessereq, _if_greatereq, _if_less, _if_greater, _call, _param, _ret, \
	_getretval, _funcstart, _funcend, _tablecreate, _tablegetelem, _tablesetelem, _jump, _nop
};

enum Expr_type { var_e, tableitem_e, programfunc_e, libfunc_e, arithm_e, bool_e, assign_e, \
	newtable_e, constnum_e, constbool_e, conststring_e, nil_e
};

enum scopespace_t { program_var, function_local, formal_argument};

struct SymbolTableEntry {
	char* name;
	uint scope;
	uint line;
	bool isActive;
	union {
		Variable* varVal;
		Function* funcVal;
	 } value;
	enum SymbolType type;
};

struct arg {
	SymbolTableEntry* entry;
	Arg_node* next;
};

struct Function {
	Arg_node* list;
	uint argc;
	uint address; /* quad index (funcstart) */
	uint local_size; /* num of function locals */
};

struct Variable {
	uint offset;
	scopespace_t space;
};

struct Expr {
	uint union_i; /* union index */
	Expr_type type;
	Expr* index; // for tables D:
	union {
		SymbolTableEntry* sym;
		double numConst;
		char* strConst;
		bool boolConst;
	} value;
	Expr* next; // for lists if needed.
};

struct Quad {
	iopcode op;
	Expr* result;
	Expr* arg1;
	Expr* arg2;
	uint label; // goto
	uint line;
	bool ignore; // IGNORE QUAD
	uint qaddress; /* debug purpose */
	uint taddress; /* target code address */
};

#endif /* _DEFINITIONS_H_ */
