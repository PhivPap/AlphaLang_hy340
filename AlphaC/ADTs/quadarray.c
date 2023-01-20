#include "quadarray.h"
#include "opt.h"

#define DEFAULT_QUADARRAY_SIZE 1024
#define PRINT_QUAD_SKIP_PART fprintf(fp_out,"                    ");

#ifdef __OPT
	#define PRINT_IGNORE_QUAD_PART 	if(quads[i].ignore) fprintf(fp_out, "IGNORE QUAD "); \
							   		else fprintf(fp_out, "            ");
#else
	#define PRINT_IGNORE_QUAD_PART 
#endif

struct QuadArray {
	uint size;
	uint currLen;
	Quad* quads;
	GarbageExpr* glist;
};

struct GarbageExpr {
	Expr* expr;
	GarbageExpr* next;
};

void QuadArray_Garbage_add(QuadArray this, Expr* expr){
	assert(this);
	GarbageExpr *garbageNode = malloc(sizeof(struct GarbageExpr));
	garbageNode->expr = expr;
	garbageNode->next = this->glist;
	this->glist = garbageNode;	
}

static void QuadArray_Garbage_Collect(QuadArray this){
	assert(this);
	GarbageExpr *prev, *curr = this->glist;
	while(curr){
		prev = curr;
		curr = curr->next;
		if(prev->expr->type == conststring_e)
			free(prev->expr->value.strConst);
		free(prev->expr);
		free(prev);
	}
}

QuadArray QuadArray_new(){
	QuadArray this = malloc(sizeof(struct QuadArray));
	this->quads = malloc(sizeof(struct Quad) * DEFAULT_QUADARRAY_SIZE);
	this->size = DEFAULT_QUADARRAY_SIZE;
	this->currLen = 0;
	this->glist = NULL;
	return this;
}

void QuadArray_free(QuadArray this){
	assert(this);
	QuadArray_Garbage_Collect(this);
	free(this->quads);
	free(this);
}

Quad* QuadArray_getQuads(QuadArray this){
	assert(this);
	return this->quads;
}

uint QuadArray_nextQuad(QuadArray this){
	assert(this);
	return this->currLen;
}

uint QuadArray_getSize(QuadArray this){
	assert(this);
	return this->size;
}

static void QuadArray_expand(QuadArray this){
	assert(this);
	assert(this->size == this->currLen);
	this->size += DEFAULT_QUADARRAY_SIZE;
	this->quads = realloc(this->quads, sizeof(struct Quad) * this->size);
}

void QuadArray_insert_quad(QuadArray this, iopcode op, Expr *arg1, Expr *arg2, Expr *result, int label, uint line){
	assert(this);
	if(this->currLen == this->size)
		QuadArray_expand(this);
	Quad* p = &(this->quads[this->currLen++]);

	p->op = op;
	p->arg1 = arg1;
	p->arg2 = arg2;
	p->result = result;
	p->label = label;
	p->line = line;
	p->ignore = 0;
	
	//p->qaddress = this->currLen - 1;
}

void QuadArray_patchLabel(QuadArray this, uint index, int label){
	assert(this);
	assert((index >= 0) && (index < this->currLen));
	this->quads[index].label = label;
}

void QuadArray_patchLabel_increase(QuadArray this, uint index, int labelIncrease){
	assert(this);
	assert((index >= 0) && (index < this->currLen));
	this->quads[index].label += labelIncrease;
}

int QuadArray_getLabel(QuadArray this, uint index){
	assert(this);
	assert((index >= 0) && (index < this->currLen));
	return this->quads[index].label;
}

iopcode QuadArray_getOp(QuadArray this, uint index){
	assert(this);
	if(index == -1) return this->quads[0].op; //hard patch for x = const op const ..op const case 
	assert((index >= 0) && (index < this->currLen));
	return this->quads[index].op;
}

static void printExpr(Expr *expr, FILE* fp_out){
	if(!expr){
		fprintf(fp_out, "%-12c", ' ');
		return;
	}
	char *tmp;
	switch(expr->type){
		case var_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name);
			break;
		case tableitem_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name); // ??
			break;
		case programfunc_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name); // ??
			break;
		case libfunc_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name); // ??
			break;
		case arithm_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name); //????????????????????????????????????????????????????????
			break;
		case bool_e:
			fprintf(fp_out,"%-12s", expr->value.sym->name);
			break;
		case assign_e:
			assert(0); // todo
			break;
		case newtable_e:
			assert(0); // todo
			break;
		case constnum_e:
			fprintf(fp_out,"%-12g", expr->value.numConst);
			break;
		case constbool_e:
			if(expr->value.boolConst == 0)
				fprintf(fp_out,"%-12s", "false");
			else
				fprintf(fp_out,"%-12s", "true");
			break;
		case conststring_e:
			tmp = malloc(strlen(expr->value.strConst) + 3);
			tmp[0] = '\"';
			strcpy(&tmp[1], expr->value.strConst);
			tmp[strlen(expr->value.strConst) + 1] = '\"';
			tmp[strlen(expr->value.strConst) + 2] = '\0';
			fprintf(fp_out,"%-12s", tmp);
			free(tmp);
			break;
		case nil_e:
			fprintf(fp_out,"%-12s", "nil");
			break;
		default:
			assert(0);

	}
}
// ?   savidis gitgud

void QuadArray_write_quads(QuadArray this){
	assert(this);
	#ifdef __QSTDOUT
		FILE* fp_out = stdout;
	#else
		FILE* fp_out = fopen("quads.txt", "w+");
	#endif
	Quad* quads = this->quads;
	uint currQuad = this->currLen;
	for(int i = 0; i < currQuad; i++){
		fprintf(fp_out,"Q %-6d  %-12s  ", i, iopcodeToString[quads[i].op]);
		switch(quads[i].op){
			case _ret:
				printExpr(quads[i].result, fp_out); //part1
				fprintf(fp_out,"        ");
				PRINT_QUAD_SKIP_PART //part2
				PRINT_QUAD_SKIP_PART //part3
				PRINT_IGNORE_QUAD_PART //part4
				break;
			case _call:
			case _param:
			case _getretval:
			case _funcstart:
			case _funcend:
				printExpr(quads[i].result, fp_out); //part1
				fprintf(fp_out,"        ");
				PRINT_QUAD_SKIP_PART //part2
				PRINT_QUAD_SKIP_PART //part3
				PRINT_IGNORE_QUAD_PART
				break;
			case _jump:
				fprintf(fp_out,"JUMPTO: %-9d   ", quads[i].label); //part1
			 	PRINT_QUAD_SKIP_PART //part2
				PRINT_QUAD_SKIP_PART //part3
				PRINT_IGNORE_QUAD_PART
				break;
			case _assign:
			case _uminus:
			case _not:
				fprintf(fp_out,"RES: "); //part1
				printExpr(quads[i].result, fp_out);
				fprintf(fp_out,"   ");
				fprintf(fp_out,"ARG1: "); //part2
				printExpr(quads[i].arg1, fp_out);
				fprintf(fp_out,"  ");
				PRINT_QUAD_SKIP_PART //part3
				PRINT_IGNORE_QUAD_PART
				break;
			case _add:
			case _sub:
			case _mul:
			case _div:
			case _mod:
			case _and:
			case _or:
				fprintf(fp_out,"RES: "); //part1
				printExpr(quads[i].result, fp_out);
				fprintf(fp_out,"   ");
				fprintf(fp_out,"ARG1: "); //part2
				printExpr(quads[i].arg1, fp_out);
				fprintf(fp_out,"  ");
				fprintf(fp_out,"ARG2: "); //part3
				printExpr(quads[i].arg2, fp_out);
				fprintf(fp_out,"  ");
				PRINT_IGNORE_QUAD_PART
				break;
			case _if_eq:
			case _if_noteq:
			case _if_lessereq:
			case _if_greatereq:
			case _if_less:
			case _if_greater:
				fprintf(fp_out,"JUMPTO: %-9d   ", quads[i].label); //part1
				fprintf(fp_out,"ARG1: "); //part2
				printExpr(quads[i].arg1, fp_out);
				fprintf(fp_out,"  ");
				fprintf(fp_out,"ARG2: "); //part3
				printExpr(quads[i].arg2, fp_out);
				fprintf(fp_out,"  ");
				PRINT_IGNORE_QUAD_PART
				break;
			case _tablegetelem: // <------- wot ?
				fprintf(fp_out,"GET: "); //part1
				printExpr(quads[i].result, fp_out);
				fprintf(fp_out,"   ");
				fprintf(fp_out,"TABLE: "); //part2
				printExpr(quads[i].arg1, fp_out);
				fprintf(fp_out," ");
				fprintf(fp_out,"ITEM: "); //part3
				printExpr(quads[i].arg2, fp_out);
				fprintf(fp_out,"  ");
				PRINT_IGNORE_QUAD_PART
				break;
			case _tablesetelem: // <------- wot ?
				fprintf(fp_out,"SET: "); //part1
				printExpr(quads[i].result, fp_out);
				fprintf(fp_out,"   ");
				fprintf(fp_out,"TABLE: "); //part2
				printExpr(quads[i].arg1, fp_out);
				fprintf(fp_out," ");
				fprintf(fp_out,"ITEM: "); //part3
				printExpr(quads[i].arg2, fp_out);
				fprintf(fp_out,"  ");
				PRINT_IGNORE_QUAD_PART
				break;
			case _tablecreate:
				printExpr(quads[i].result, fp_out);
				fprintf(fp_out,"        ");
				PRINT_QUAD_SKIP_PART
				PRINT_QUAD_SKIP_PART
				PRINT_IGNORE_QUAD_PART
				break;
			default: assert(0);
		}
		fprintf(fp_out,"  [line %u]\n", quads[i].line); //part4
	}
	fprintf(fp_out,"\n");
	#ifndef __QSTDOUT
		fclose(fp_out);
	#endif
}

#ifdef __OPT

void QuadArray_deadcode_elimination(QuadArray this,uint deadcodeIndex){
	assert(this);
	Quad* quads = this->quads;
	run_optimization_code(quads,this->currLen,deadcodeIndex);	
}

void QuadArray_final_opt(QuadArray this){
	assert(this);
	Quad* quads = this->quads;
	run_final_optimization_code(quads,this->currLen);
}

#endif

void QuadArray_setIgnore(QuadArray this, uint index){
	assert(this);
	this->quads[index].ignore = 1;
}

void QuadArray_patchResult(QuadArray this, uint index, Expr* res){
	assert(this);
	assert((index >= 0) && (index < this->currLen));
	this->quads[index].result = res;
}