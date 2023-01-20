#include "definitions.h"

typedef struct QuadArray * QuadArray;
typedef struct GarbageExpr GarbageExpr;

void QuadArray_Garbage_add(QuadArray this, Expr* expr);

QuadArray QuadArray_new();

void QuadArray_free(QuadArray this);

Quad* QuadArray_getQuads(QuadArray this);

uint QuadArray_nextQuad(QuadArray this);

uint QuadArray_getSize(QuadArray this);

void QuadArray_insert_quad(QuadArray this, iopcode op, Expr *arg1, Expr *arg2, Expr *result, int label, uint line);

void QuadArray_patchLabel(QuadArray this, uint index, int label);

void QuadArray_patchLabel_increase(QuadArray this, uint index, int labelIncrease);

int QuadArray_getLabel(QuadArray this, uint index);

iopcode QuadArray_getOp(QuadArray this, uint index);

void QuadArray_write_quads(QuadArray this);

#ifdef __OPT
	void QuadArray_cleanup(QuadArray this);
	void QuadArray_deadcode_elimination(QuadArray this, uint currQuad);
	void QuadArray_final_opt(QuadArray this);
/*#else
	#define QuadArray_cleanup(...)
	#define QuadArray_deadcode_elimination(...)
	#define QuadArray_final_opt(...)*/
#endif

void QuadArray_setIgnore(QuadArray this,  uint index);

void QuadArray_patchResult(QuadArray this, uint index, Expr* res);

