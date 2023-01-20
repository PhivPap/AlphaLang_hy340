#include "opt.h"

usedTempVars usedtmplist;

struct usedTempVars {
	char* name;
	usedTempVars next;
	uint quad_index; /* debugging reasons */
};

static unsigned char contains_tmpVar(Expr* e){
	assert(e);
	usedTempVars ptr = usedtmplist;
	while(ptr){
		if(strcmp(ptr->name, e->value.sym->name) == 0)
			return 1;
		ptr = ptr->next;
	}
	return 0;
}

static void insert_usedTmpVar(Expr* e, uint index){
	assert(e);
	usedTempVars new = malloc(sizeof(struct usedTempVars));
	new->name = e->value.sym->name;
	new->next = usedtmplist;
	new->quad_index = index;
	usedtmplist = new;
}

static void printTmpVarList() {
	usedTempVars curr = usedtmplist;
	
	printf("\n\n------- Useful tmp vars -------\n\n");
	
	while(curr){
		printf("Q %u: %s\n",curr->quad_index, curr->name);
		curr = curr->next;		
	}
	printf("\n");
}

static void freeTmpVarList() {
	usedTempVars curr = usedtmplist;

	while(curr){
		usedTempVars prev = curr;
		curr = curr->next;
		free(prev);
	}
	free(curr);
}

/* keep in mind traversal should be done in reverse */
static void ignore_and_set_qaddress(Quad* quads, uint i, uint currQuad){
	assert(quads);
	quads[i].ignore = 1;

	/* watchout */
	/*if(i == currQuad - 1) // last quad.
		quads[i].qaddress += 1;
	else*/ 
		quads[i].qaddress = quads[i+1].qaddress;
	
}

static void ignoreConditionalCode(Quad* quads,uint from, uint to) {
	assert(quads);
	for(int i = to - 1; i >= (int)from; i--)
		ignore_and_set_qaddress(quads,i,to);
}

static unsigned char areOperandsConstbools(Expr* e1, Expr* e2){
	assert(e1);
	assert(e2);
	return (e1->type == constbool_e && e2->type == constbool_e) ? 1 : 0;
}

static void deadcode_optimization(Quad* quads, uint currQuad, uint deadcodeIndex){
	assert(quads);
	/*
		QUAD TYPES:
		assert 0 (default): _and, _or, _not, _nop.
		>>do nothin(??): _jump,  _funcstart, _funcend. (_tablecreate) for now.
		!!!dunno (prob nothing): _tablecreate
		>>sets result:  _getretval.
		>>uses result: _call, _param, _ret.
		sets result and uses arg1: _assign, _uminus.
		uses arg1, arg2: _if_eq, _if_noteq, _if_lessereq, _if_greatereq, _if_less, _if_greater.
		sets result and uses arg1, arg2: _add, _sub, _mul, _div, _mod, _tablegetelem
		uses result, arg1, arg2: _tablesetelem
	*/
	for(int i = currQuad - 1; i >= (int)deadcodeIndex; i--){
		
		switch(quads[i].op) {
			case _jump:
			case _funcstart:
			case _funcend: 
			case _tablecreate:
				break;

			case _getretval:
				assert(quads[i].result);
				if(isTempExpr(quads[i].result)){
					if(!contains_tmpVar(quads[i].result))
						ignore_and_set_qaddress(quads,i,currQuad);
				break;
	
			case _call:
			case _param:
			case _ret:
				if(quads[i].result != NULL){
					if(isTempExpr(quads[i].result))
						insert_usedTmpVar(quads[i].result, quads[i].qaddress);
				}

				/*assert(quads[i].result);
				if(isTempExpr(quads[i].result))
					insert_usedTmpVar(quads[i].result, quads[i].qaddress);*/
				break;

			case _assign:
			case _uminus:
				assert(quads[i].result);
				assert(quads[i].arg1);
				if(isTempExpr(quads[i].result)){
					if(!contains_tmpVar(quads[i].result)){
						ignore_and_set_qaddress(quads,i,currQuad);
					}
					else{
						if(isTempExpr(quads[i].arg1))
							insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
					}
				}
				else{
					if(isTempExpr(quads[i].arg1))
						insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
				}
				break;

			case _if_eq:
				assert(quads[i].arg1);
				assert(quads[i].arg2);
				if(areOperandsConstbools(quads[i].arg1, quads[i].arg2)){
					/* if_eq -> true */
					if(quads[i].arg1->value.boolConst == quads[i].arg2->value.boolConst)
						ignoreConditionalCode(quads, i, quads[i].label);
					/* if_eq -> false */
					else
						ignoreConditionalCode(quads, i, quads[i+1].label);
					break;
				}
				/* else fall down .. */
			case _if_noteq:
				if(areOperandsConstbools(quads[i].arg1, quads[i].arg2)){
					if(quads[i].arg1->value.boolConst != quads[i].arg2->value.boolConst)
						ignoreConditionalCode(quads, i, quads[i].label);
					break;
				}
			case _if_lessereq:
			case _if_greatereq:
			case _if_less:
			case _if_greater:
				assert(quads[i].arg1);
				assert(quads[i].arg2);
				if(isTempExpr(quads[i].arg1))
					insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
				if(isTempExpr(quads[i].arg2))
					insert_usedTmpVar(quads[i].arg2, quads[i].qaddress);
				break;

			case _add:
			case _sub:
			case _mul:
			case _div:
			case _mod:
			case _tablegetelem:
				assert(quads[i].result);
				assert(quads[i].arg1);
				assert(quads[i].arg2);
				if(isTempExpr(quads[i].result)){
					if(!contains_tmpVar(quads[i].result))
						ignore_and_set_qaddress(quads,i,currQuad);
					else{
						if(isTempExpr(quads[i].arg1))
							insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
						if(isTempExpr(quads[i].arg2))
							insert_usedTmpVar(quads[i].arg2, quads[i].qaddress);
					}
				}
				else{
					if(isTempExpr(quads[i].arg1))
						insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
					if(isTempExpr(quads[i].arg2))
						insert_usedTmpVar(quads[i].arg2, quads[i].qaddress);
				}
				break;

			case _tablesetelem:
				assert(quads[i].result);
				assert(quads[i].arg1);
				assert(quads[i].arg2);
				if(isTempExpr(quads[i].arg1))
					insert_usedTmpVar(quads[i].arg1, quads[i].qaddress);
				if(isTempExpr(quads[i].arg2))
					insert_usedTmpVar(quads[i].arg2, quads[i].qaddress);
				if(isTempExpr(quads[i].result))
					insert_usedTmpVar(quads[i].result, quads[i].qaddress);
				break;

			default:
				assert(0);				
		}
	}
}}

static uint sequencedJumpsPatch(Quad* quads, uint index){

	if(!quads || quads[index].op != _jump)
		return index; /* return previous jump quad label */

	uint jump_label = sequencedJumpsPatch(quads, quads[index].label);

	quads[index].label = jump_label; /* actual patch */
	return jump_label;
}

static void jump_optimization(Quad* quads, uint currQuad){
	assert(quads);

	for(int i = 0; i < currQuad; i++)
		if(quads[i].op == _jump)
			sequencedJumpsPatch(quads,i);
}

static void patch_ignore_jumps(Quad* quads, uint currQuad) {
	assert(quads);

	for(int i = 0; i < (int)currQuad - 1 ; i++) {

		switch(quads[i].op) {
		/* all label opcodes */
		case _jump:
			if(quads[quads[i].label].ignore == 1)
				quads[i].label = quads[quads[i].label].qaddress;
			break;
		case _if_eq:
		case _if_noteq:
		case _if_greater:
		case _if_greatereq:
		case _if_less:
		case _if_lessereq:
			
			if(quads[quads[i].label].ignore == 1)
				quads[i].label = quads[quads[i+1].label].qaddress;
		default: return;
		}	
	}
}

/* ran during intermediate code generation */
void run_optimization_code(Quad* quads, uint currQuad, uint deadcodeIndex){
	assert(quads);
	usedtmplist = NULL;
	deadcode_optimization(quads, currQuad, deadcodeIndex);
	//printTmpVarList();
	freeTmpVarList();
}

/* ran only when finished with intermediate code generation */
void run_final_optimization_code(Quad* quads, uint currQuad){
	assert(quads);
	
	//patch_ignore_jumps(quads,currQuad);
	//jump_optimization(quads,currQuad);
}