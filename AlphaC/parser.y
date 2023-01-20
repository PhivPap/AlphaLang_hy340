%{
	//HY-340 phase3 csd3737, csd3823
	#include "ADTs/definitions.h"
	#include "ADTs/scopelist.h"
	#include "ADTs/symtable.h"
	#include "ADTs/quadarray.h"
	#include "ADTs/logiclist.h"
	#include "ADTs/instrarray.h"
	#include <time.h>
	#include <unistd.h>

	#define LIB_FUNC_AMOUNT 12
	#define emit(args...) QuadArray_insert_quad(quads, ##args, yylineno)

	extern FILE *yyin;
	extern int yylineno;
	extern char *yytext;

	SymTable_T symtable;
	ScopeList list;
	QuadArray quads;
	LogicList logiclist;

	typedef struct alpha_token_t token;
	
	struct alpha_token_t {
		uint numline;
		uint numToken;
		char* content;
		char* type;
	};

	typedef struct indexedelem_pair indexed_pair;
	
	struct indexedelem_pair {
		Expr* index;
		Expr* value;
		indexed_pair* next;
	};

	typedef struct call_node call_node;

	struct call_node {
		Expr* elist;
		unsigned char method; 
		char* name;
	};


	int yyerror(char* yaccProvidedMessage);
	int yylex();

	Expr* getResultExpr(Expr* expr1, Expr* expr2, iopcode op);
	Expr* computeConstOpConst(Expr* e1, Expr *e2, iopcode op);
	void checkExprOpExpr(Expr* e1, Expr* e2, iopcode op);
	void updateAnonymVarC(Expr* e1, Expr* e2);
	double strToDouble(char* str);
	Expr* new_const_expr(Expr_type type, void* val);
	Expr* new_lvalue_expr(SymbolTableEntry* entry);
	void emit_args(Expr* func, Expr* arg);
	void checkRValueWrite(Expr* expr);
	void printRule(const char* after, const char* before);
	void printTok(token* tok);
	SymbolTableEntry* referenceGlobal(token* tok);
	SymbolTableEntry* createLocal(token* tok);
	SymbolTableEntry* createEntry(token* tok, enum SymbolType type);
	void increaseScope();
	void decreaseScope(int dontHideScope);
	SymbolTableEntry* insertVar(token* tok);
	void insertArg(token* tok);
	SymbolTableEntry* insertFunc(token* tok);
	void flagError(void);
	void insertEntry(SymbolTableEntry* STEntry);
	void initializeLibFunc(char* name);
	char* nameIsLibTaken(char* name);
	token createAnonymousFunction();
	SymbolTableEntry* insertAnonymVar();
	void resetAnonymVars();
	void patchReturn(int from, int to);
	int isInsideFunction();
	void patchBreakContinue(int from, int to);
	void boolConversion(Expr* expr);
	void BackPatchBoolean(Expr* result);
	LogicList createTmpLogicList();
	Expr* member_item(Expr* lvalue, char* content);
	Expr* emit_get_if_table_item(Expr* expr);
	double recursive_emit_tablesetelem(Expr* arg,Expr* t);
	Expr* make_call(Expr* lv, Expr* arg);

	uint currScope = 0;
	int errorFlag = 0;
	int functionScoped = 0;
	Function* lastFunction = NULL;
	uint anonymVarCounter = 0;
	uint programVarOffset = 0;
	int functionLocalOffset = -1;
	int insideLoop = 0;
	char* Temp_ID = NULL;

	char* libFuncsNames[LIB_FUNC_AMOUNT] = {"print", "input", "objectmemberkeys", "objecttotalmembers", "objectcopy", \
	 "totalarguments", "argument", "typeof", "strtonum","sqrt", "cos", "sin" };

%}

%union {
  struct alpha_token_t* tok;
  struct Expr* expr;
  struct LogicList* llist;
  struct indexedelem_pair* idxpair;
  struct call_node* cnode;
  int intval;
}

%expect 1
%start program

%token <tok> IF ELSE WHILE FOR FUNCTION RETURN BREAK CONTINUE LOCAL
		TRUE FALSE NIL INTCONST REALCONST STRING ID CURLY_BR_O CURLY_BR_C
		COLON COMMA DBL_COLON SEMICOLON

%right <tok> ASSIGN
%left <tok> OR
%left <tok> AND
%nonassoc <tok> EQUAL UNEQUAL
%nonassoc <tok> GREATER_THAN GREATER_EQUAL LESSER_THAN LESSER_EQUAL
%left  <tok> PLUS MINUS
%left  <tok> ASTERISK SLASH MODULO
%right <tok> NOT PLUS_PLUS MINUS_MINUS
%right <tok> UMINUS
%left <tok> PERIOD DBL_PERIOD
%left <tok> SQUARE_BR_O SQUARE_BR_C
%left <tok> ROUND_BR_O ROUND_BR_C

%type <expr> lvalue member primary term expr assignexpr const elist call funcid
%type <expr> funcprefix funcdef optionalexpr objectdef objectdeftemp
%type <intval> block funcdecl ifheader elseheader elsestmt forelist1 forexpr forstart whilestart whilecond
%type <llist> newllist
%type <idxpair> indexed indexedelem
%type <cnode> callsuffix normcall methodcall dummy


%%

program:		program stmt { resetAnonymVars(); }
				|  {}
			
stmt:			expr SEMICOLON { BackPatchBoolean($1);}
				| ifstmt {}
				| whilestmt {}
				| forstmt {}
				| returnstmt {}
				| BREAK SEMICOLON {
					emit(_jump, NULL, NULL, NULL, -2);
					if(!insideLoop){
						printf("(line %u): break statement not in loop.\n", yylineno);
						flagError();
					}
				}

				| CONTINUE SEMICOLON {
						emit(_jump, NULL, NULL, NULL, -3);
						if(!insideLoop){
							printf("(line %u): continue statement not in loop.\n", yylineno);
						flagError();
					}
				}

				| block {}
				| funcdef {}
				| SEMICOLON {}

expr:	 		assignexpr
				| expr PLUS expr {
					$$ = getResultExpr($1, $3, _add);
					if($$->type != constnum_e)
						emit(_add, $1, $3, $$, 0);	
				}

				| expr MINUS expr {
					$$ = getResultExpr($1, $3, _sub);
					if($$->type != constnum_e)
						emit(_sub, $1, $3, $$, 0);
				}

			 	| expr ASTERISK expr {
					$$ = getResultExpr($1, $3, _mul);
					if($$->type != constnum_e)
						emit(_mul, $1, $3, $$, 0);
				}

			 	| expr SLASH expr {
					$$ = getResultExpr($1, $3, _div);
					if($$->type != constnum_e)
						emit(_div, $1, $3, $$, 0);
				}

			 	| expr MODULO expr {
					$$ = getResultExpr($1, $3, _mod);
					if($$->type != constnum_e)
						emit(_mod, $1, $3, $$, 0);
				}

			 	| expr GREATER_THAN expr {
					$$ = getResultExpr($1, $3, _if_greater);
					emit(_if_greater, $1, $3, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;
				}

			 	| expr GREATER_EQUAL expr {
					$$ = getResultExpr($1, $3, _if_greatereq);
					emit(_if_greatereq, $1, $3, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;
				}

			 	| expr LESSER_THAN expr {
					$$ = getResultExpr($1, $3, _if_less);
					emit(_if_less, $1, $3, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;
				}

			 	| expr LESSER_EQUAL expr {
					$$ = getResultExpr($1, $3, _if_lessereq);
					emit(_if_lessereq, $1, $3, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;	
				}

			 	| expr EQUAL {BackPatchBoolean($1);} newllist expr {
					BackPatchBoolean($5);
					logiclist = LogicList_merge(logiclist, $4);
					$$ = getResultExpr($1, $5, _if_eq);
					emit(_if_eq, $1, $5, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;
				}

			 	| expr UNEQUAL {BackPatchBoolean($1);} newllist expr {		
					BackPatchBoolean($5);
					logiclist = LogicList_merge(logiclist, $4);
					$$ = getResultExpr($1, $5, _if_noteq);
					emit(_if_noteq, $1, $5, NULL, -4);
					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
					emit(_jump, NULL, NULL, NULL, -5);
					LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
					$$->type = bool_e;
				}

			 	| expr AND {
						if($1->type != bool_e){
							unsigned char tr = 1;
							emit(_if_eq, $1, new_const_expr(constbool_e, &tr), NULL, QuadArray_nextQuad(quads)+2);
							emit(_jump, NULL, NULL, NULL, -5);
							LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
						}
						else
			 				LogicList_backPatch_true(logiclist, quads, QuadArray_nextQuad(quads));	 		
					}
					newllist expr {
						$$ = getResultExpr($1, $5, _and);
						if($5->type != bool_e){
							unsigned char tr = 1;
							emit(_if_eq, $5, new_const_expr(constbool_e, &tr), NULL, -4);
							LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
							emit(_jump, NULL, NULL, NULL, -5);
							LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
						}
						logiclist = LogicList_merge(logiclist, $4);
						$$->type = bool_e;
				}

			 	| expr OR  {
	 	 				if($1->type != bool_e){
	 	 					unsigned char tr = 1;
	 	 					emit(_if_eq, $1, new_const_expr(constbool_e, &tr) ,NULL, -4);
	 	 					LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
	 	 					emit(_jump, NULL, NULL, NULL, QuadArray_nextQuad(quads)+1);
	 	 				}
	 	 				else
	 	 					LogicList_backPatch_false(logiclist, quads, QuadArray_nextQuad(quads)); 	 				
	 				}
	 				newllist expr {
						$$ = getResultExpr($1, $5, _or);
						if($5->type != bool_e){
							unsigned char tr = 1;
							emit(_if_eq, $5, new_const_expr(constbool_e, &tr), NULL, -4);
							LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
							emit(_jump, NULL, NULL, NULL, -5);
							LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
						}
						logiclist = LogicList_merge(logiclist, $4);
						$$->type = bool_e;
		 	 	}

			 	| term {$$ = $1;}


term:	 		ROUND_BR_O expr ROUND_BR_C { $$ = $2; }

				| MINUS expr %prec UMINUS {
					double val = -1;
					Expr* minus_expr = new_const_expr(constnum_e,&val);
					$$ = getResultExpr($2, minus_expr, _mul);
					if($$->type != constnum_e)
						emit(_mul, $2, minus_expr, $$, 0);
				}

				| NOT newllist expr {
					$$ = getResultExpr($3, NULL, _not);
					if($3->type != bool_e)
						boolConversion($3);
					LogicList_invert(logiclist);
					logiclist = LogicList_merge(logiclist, $2);
					$$->type = bool_e;
				}

				| PLUS_PLUS lvalue { // todo
					checkRValueWrite($2);
					double num = 1;
					Expr *constExpr = new_const_expr(constnum_e, &num);
					if($2->type == tableitem_e) {
						$$ = emit_get_if_table_item($2);
						emit(_add, $$, constExpr, $$, 0);
						emit(_tablesetelem, $2, $2->index, $$, 0);
					}
					else {
						emit(_add, $2, constExpr, $2, 0);
						SymbolTableEntry * entry = insertAnonymVar();
						$$ = new_lvalue_expr(entry);
						emit(_assign, $2, NULL, $$, 0);
					}
				}
				
				| lvalue PLUS_PLUS {
					checkRValueWrite($1);
					SymbolTableEntry * entry = insertAnonymVar();
					$$ = new_lvalue_expr(entry);
					double num = 1;
					Expr *constExpr = new_const_expr(constnum_e, &num);

					Expr * var = emit_get_if_table_item($1);
					emit(_assign, var, NULL, $$, 0);
					emit(_add, var, constExpr, var, 0);
					if($1->type == tableitem_e){
						emit(_tablesetelem, $1, $1->index, var, 0);
					}

				}

		 		| MINUS_MINUS lvalue {
					checkRValueWrite($2);
					double num = 1;
					Expr *constExpr = new_const_expr(constnum_e, &num);
					if($2->type == tableitem_e) {
						$$ = emit_get_if_table_item($2);
						emit(_sub, $$, constExpr, $$, 0);
						emit(_tablesetelem, $2, $2->index, $$, 0);
					}
					else {
						emit(_sub, $2, constExpr, $2, 0);
						SymbolTableEntry * entry = insertAnonymVar();
						$$ = new_lvalue_expr(entry);
						emit(_assign, $2, NULL, $$, 0);
					}
				}

		 		| lvalue MINUS_MINUS { 
					checkRValueWrite($1);
					SymbolTableEntry * entry = insertAnonymVar();
					$$ = new_lvalue_expr(entry);
					double num = 1;
					Expr *constExpr = new_const_expr(constnum_e, &num);

					Expr* var = emit_get_if_table_item($1);
					emit(_assign, var, NULL, $$, 0);
					emit(_sub, var, constExpr, var, 0);
					if($1->type == tableitem_e)
						emit(_tablesetelem, $1, $1->index, var, 0);
				}

				| primary {$$ = $1;}


newllist: 		{ $$ = createTmpLogicList(); }

assignexpr: 	lvalue ASSIGN expr {
					BackPatchBoolean($3);
					checkRValueWrite($1);
				

					if($1->type == tableitem_e){ // <-new
						emit(_tablesetelem, $1, $1->index, $3, 0);
						$$ = emit_get_if_table_item($1);
					}
					else{

						SymbolTableEntry* entry = insertAnonymVar();
						$$ = new_lvalue_expr(entry);

						emit(_assign, $3, NULL, $1, 0);
						emit(_assign, $1, NULL, $$, 0);
					}
				}

primary:		lvalue { $$ = emit_get_if_table_item($1); }
				| call { $$ = $1;}
				| objectdef {$$ = $1;}
				| ROUND_BR_O funcdef ROUND_BR_C {$$ = $2;}
				| const {$$ = $1;}

lvalue: 		ID {
					SymbolTableEntry *entry = insertVar($1);
					$$ = new_lvalue_expr(entry);
				}

				| LOCAL ID {
					SymbolTableEntry *entry = createLocal($2);
					$$ = new_lvalue_expr(entry);
				}

				| DBL_COLON ID {
					SymbolTableEntry *entry = referenceGlobal($2);
					$$ = new_lvalue_expr(entry);
				}

				| member { $$ = $1;}

member: 		lvalue PERIOD ID { $$ = member_item($1, $3->content); }
				
				| lvalue SQUARE_BR_O expr SQUARE_BR_C {
					//printf("expr = %s\n", $3->value.sym->name);
					//updateAnonymVarC($3,NULL);
					BackPatchBoolean($3);
					$1 = emit_get_if_table_item($1);
					$$ = new_lvalue_expr($1->value.sym);
					$$->type = tableitem_e;
					$$->index = $3;
				}

				| call PERIOD ID { $$ = member_item($1, $3->content); }
				| call SQUARE_BR_O expr SQUARE_BR_C {
					//printf("expr = %s\n", $3->value.sym->name);
					//updateAnonymVarC($3,NULL);
					BackPatchBoolean($3);
					$1 = emit_get_if_table_item($1);
					$$ = new_lvalue_expr($1->value.sym);
					$$->type = tableitem_e;
					$$->index = $3;
				}

call:			call normcall {
					$$ = make_call($$,$2->elist);
					free($2);
				}

				| lvalue callsuffix {
					
					$1 = emit_get_if_table_item($1);

					if($2->method){
						Expr* self = $1;
						self->next = NULL;
						$1 = emit_get_if_table_item(member_item(self, $2->name)); /*last dot slot is param*/
						/* guess what .. self should be added to tail of list .. */
						Expr* head = $2->elist;

						if(!head)
							$2->elist = self;
						else {
							while(head->next)
								head = head->next;
							head->next = self;
						}
					}
					$$ = make_call($1, $2->elist);
					if($2->name)
						free($2->name);
					free($2);
				}					

				| ROUND_BR_O funcdef ROUND_BR_C normcall {
					
					SymbolTableEntry* entry = insertAnonymVar();
					Expr* func = new_lvalue_expr($2->value.sym);
					func->type = programfunc_e;
					$$ = make_call(func, $4->elist);
					free($4);
				}

callsuffix:		normcall { $$ = $1;}
				| methodcall { $$ = $1;}

normcall:		ROUND_BR_O elist ROUND_BR_C {
					call_node* new_clist = malloc(sizeof(struct call_node));
					new_clist->elist = $2;
					new_clist->method = 0;
					new_clist->name = NULL;
					$$ = new_clist;
				}				

methodcall:		DBL_PERIOD dummy ID {
						call_node* new_clist = malloc(sizeof(struct call_node));
						new_clist->name = strdup($3->content);
						$2 = new_clist;
					} ROUND_BR_O elist ROUND_BR_C { 
						$2->elist = $6;
						$2->method = 1;
						$$ = $2;
				}

dummy: { $$ = NULL; }

elist:			elist COMMA expr { BackPatchBoolean($3); $3->next = $1; $$ = $3;}
				| expr { BackPatchBoolean($1); $$ = $1; $$->next = NULL;}
				| { $$ = NULL;}

objectdef:		SQUARE_BR_O objectdeftemp SQUARE_BR_C { $$ = $2;}

objectdeftemp:	elist {
					SymbolTableEntry* entry = insertAnonymVar();
					Expr* t = new_lvalue_expr(entry);
					emit(_tablecreate, NULL, NULL, t, 0);
					recursive_emit_tablesetelem($1,t);
					$$ = t;
				}

				| indexed {
					SymbolTableEntry* entry = insertAnonymVar();
					Expr* t = new_lvalue_expr(entry);
					emit(_tablecreate, NULL, NULL, t, 0);
					for(; $1; $1 = $1->next)
						emit(_tablesetelem, t, $1->index, $1->value, 0);
					$$ = t;
				}

indexed:		indexed COMMA indexedelem {
					$$ = $1;
					while($1->next)
						$1 = $1->next;
					$1->next = $3;
				}

				| indexedelem { $$ = $1;}

indexedelem:	CURLY_BR_O expr { BackPatchBoolean($2); } COLON expr CURLY_BR_C {
					BackPatchBoolean($5);
					indexed_pair* new_pair = malloc(sizeof(struct indexedelem_pair));
					new_pair->index = $2;
					new_pair->value = $5;
					new_pair->next = NULL;
					$$ = new_pair;
				}

block:			CURLY_BR_O { increaseScope();} blocktemp CURLY_BR_C { $$ = functionLocalOffset; decreaseScope(0);}

blocktemp:		blocktemp stmt {}
				| {}

funcid:			ID { 
					SymbolTableEntry *entry = insertFunc($1);
					$$ = new_lvalue_expr(entry);
				}

				| {	
					token tok = createAnonymousFunction();
					SymbolTableEntry *entry = insertFunc(&tok);
					free(tok.content); // fixed free.
					$$ = new_lvalue_expr(entry);
				}
			
funcdef:		funcdecl funcprefix funcargs block {
					$2->value.sym->value.funcVal->local_size = $4;
					emit(_funcend, NULL, NULL, $2, 0);
					assert(QuadArray_getLabel(quads, $1) == -1);
					QuadArray_patchLabel(quads, $1, QuadArray_nextQuad(quads));
					$$ = $2;
					patchReturn($1 + 2, QuadArray_nextQuad(quads)-1);
				}

funcdecl:		FUNCTION {
					emit(_jump, NULL, NULL, NULL, -1);
					$$ = QuadArray_nextQuad(quads) - 1;
				}

funcprefix:		funcid {
					$$ = $1;
					emit(_funcstart, NULL, NULL, $1, 0);
				}

funcargs:		ROUND_BR_O { increaseScope(); } idlist ROUND_BR_C { decreaseScope(1);}


const:			INTCONST {
					double val = strToDouble($1->content);
 					$$ = new_const_expr(constnum_e, &val);
 				}

				| REALCONST {
					double val = strToDouble($1->content);
 					$$ = new_const_expr(constnum_e, &val);
				}

				| STRING {
					$$ = new_const_expr(conststring_e, $1->content);
					free($1->content);
				}

				| NIL {	$$ = new_const_expr(nil_e, NULL); }

				| TRUE {
					unsigned char val = 1;
					$$ = new_const_expr(constbool_e, &val);
				}

				| FALSE {
					unsigned char val = 0;
					$$ = new_const_expr(constbool_e, &val);
				}

idlist:			idlist COMMA ID { insertArg($3); }
				| ID { insertArg($1); }
				| {}

ifstmt:			ifheader stmt { 
					QuadArray_patchLabel(quads, $1, QuadArray_nextQuad(quads));
				}
				elsestmt {
					QuadArray_patchLabel_increase(quads, $1, $4);
				}

ifheader: 		IF ROUND_BR_O expr ROUND_BR_C {	
					BackPatchBoolean($3);
					unsigned char val = 1;
					emit(_if_noteq, $3, new_const_expr(constbool_e, &val), NULL, -1);
					$$ = QuadArray_nextQuad(quads) - 1;
				}

elsestmt:		elseheader stmt { QuadArray_patchLabel(quads, $1, QuadArray_nextQuad(quads)); $$ = 1;}
				| { $$ = 0;}

elseheader: 	ELSE {
					emit(_jump, NULL, NULL, NULL, -1);
					$$ = QuadArray_nextQuad(quads) - 1;
				}


whilestmt:		whilestart whilecond stmt {
					insideLoop--;
					emit(_jump, NULL, NULL, NULL, $1);
					assert(QuadArray_getLabel(quads, $2) == -1);
					QuadArray_patchLabel(quads, $2, QuadArray_nextQuad(quads));
					patchBreakContinue($1, QuadArray_nextQuad(quads));
				}

whilestart:		WHILE {
					insideLoop++; 
					$$ = QuadArray_nextQuad(quads);
				}

whilecond:		ROUND_BR_O expr ROUND_BR_C {
					BackPatchBoolean($2);
					unsigned char val = 1;
					emit(_if_eq, $2, new_const_expr(constbool_e, &val), NULL, QuadArray_nextQuad(quads) + 2);
					$$ = QuadArray_nextQuad(quads);
					emit(_jump, NULL, NULL, NULL, -1);
				}

forstmt:		FOR { insideLoop++; } ROUND_BR_O forelist1 forexpr elist {emit(_jump, NULL, NULL, NULL, $4);} forstart stmt {
					QuadArray_patchLabel(quads, $5, $8);
					emit(_jump, NULL, NULL, NULL, $5 + 2);
					QuadArray_patchLabel(quads, $5 + 1, QuadArray_nextQuad(quads));
					patchBreakContinue($5 + 2, QuadArray_nextQuad(quads));
					insideLoop--;
				}

forelist1:		elist SEMICOLON { $$ = QuadArray_nextQuad(quads); /* loop 2 this */}

forexpr:		expr SEMICOLON {
					BackPatchBoolean($1);
					unsigned char val = 1;
					$$ = QuadArray_nextQuad(quads); /* fix true here, fix false to $$ + 1 */
					emit(_if_eq, $1, new_const_expr(constbool_e, &val), NULL, -1);
					emit(_jump, NULL, NULL, NULL, -1);
				}

forstart:		ROUND_BR_C {
					$$ = QuadArray_nextQuad(quads);
				}
		

returnstmt:		RETURN optionalexpr SEMICOLON {
					if(!isInsideFunction()) {
						printf("(line %u): return statement not in function.\n", yylineno);
						flagError();
					}
					emit(_ret, NULL, NULL, $2, 0);
					emit(_jump, NULL, NULL, NULL, -1);
				}

optionalexpr:	expr { BackPatchBoolean($1); $$ = $1; }
				|	{ $$ = NULL; }

%%

double recursive_emit_tablesetelem(Expr* arg,Expr* t){
	if(!arg)
		return 0;

	double c = recursive_emit_tablesetelem(arg->next,t);
	emit(_tablesetelem, t, new_const_expr(constnum_e, &c), arg, 0);
	return c + 1;
}

Expr* make_call(Expr* lv, Expr* arg){

	Expr* func = emit_get_if_table_item(lv);
	uint argc = 0;
	while(arg){
		emit(_param, NULL, NULL, arg, 0);
		//updateAnonymVarC(arg,NULL);
		argc++;
		arg = arg->next;
	}
	emit(_call, NULL, NULL, func, 0);
	SymbolTableEntry* entry = insertAnonymVar();
	Expr* res = new_lvalue_expr(entry);
	emit(_getretval, NULL, NULL, res, 0);

	if(func->type == var_e){ //calling func by var. ??? reminder !
		return res;
	}

	assert(func->type == programfunc_e || func->type == libfunc_e);
	if(argc < func->value.sym->value.funcVal->argc){
		//flagError();
		printf("Warning (line %d) too few arguments to function '%s'.\n", yylineno, func->value.sym->name);
		printf("(line %u) user function declared here.\n", func->value.sym->line);
	}

	return res;
}

Expr* member_item(Expr* lvalue, char* content){
	lvalue = emit_get_if_table_item(lvalue);
	Expr* item = new_lvalue_expr(lvalue->value.sym);
	item->type = tableitem_e;
	item->index = new_const_expr(conststring_e, content);
	return item;
}

Expr* emit_get_if_table_item(Expr* expr){
	if(expr->type != tableitem_e)
		return expr;
	SymbolTableEntry* entry = insertAnonymVar();
	Expr* result = new_lvalue_expr(entry);
	emit(_tablegetelem, expr, expr->index, result, 0);
	return result;
}

LogicList createTmpLogicList(){
	LogicList new = LogicList_new();
	LogicList tmp = logiclist;
	logiclist = new;
	return tmp;
}

void BackPatchBoolean(Expr *result){
	assert(result);
	if(result->type != bool_e)
		return;
	unsigned char tr = 1;
	unsigned char fal = 0;
	emit(_assign, new_const_expr(constbool_e, &tr), NULL,result,0);
	int truePatch = QuadArray_nextQuad(quads)-1;
	emit(_jump, NULL, NULL, NULL, QuadArray_nextQuad(quads) + 2);
	emit(_assign, new_const_expr(constbool_e, &fal), NULL,result,0);
	int falsePatch = QuadArray_nextQuad(quads)-1;
	LogicList_backPatch(logiclist, quads, truePatch, falsePatch);
}

void boolConversion(Expr *expr) {
	unsigned char val = 1;
	emit(_if_eq, expr, new_const_expr(constbool_e, &val), NULL, -5);
	LogicList_insert_trueList(logiclist, QuadArray_nextQuad(quads)-1);
	emit(_jump, NULL, NULL, NULL, -4);						
	LogicList_insert_falseList(logiclist, QuadArray_nextQuad(quads)-1);
}

void patchBreakContinue(int from, int to) {
	for (int i = from; i < to; i++){
		if(QuadArray_getLabel(quads, i) == -2)
			QuadArray_patchLabel(quads, i, to);

		if(QuadArray_getLabel(quads, i) == -3)
			QuadArray_patchLabel(quads, i, from);
	}
}

int isInsideFunction(){
	return functionLocalOffset == -1 ? 0 : 1;
}

void patchReturn(int from, int to) {
	assert(from <= to);

	for(int i = from; i < to; i++)
		if(QuadArray_getOp(quads, i) == _ret){
			assert(i+1 < to);
			if(QuadArray_getLabel(quads, i+1) == -1)
				QuadArray_patchLabel( quads, i+1, to);
		}
}

Expr* getResultExpr(Expr *e1, Expr *e2, iopcode op) {
	Expr* res;
	checkExprOpExpr(e1, e2, op);
	res = computeConstOpConst(e1, e2, op);
	if(!res) {
		updateAnonymVarC(e1, e2);
		SymbolTableEntry* entry = insertAnonymVar();
		res = new_lvalue_expr(entry);
	}
	return res;
}

/* check if expr op expr is valid,else flagError and print error messages*/
void checkExprOpExpr(Expr* e1, Expr* e2, iopcode op) {
	assert(e1);
	
	switch(op) {
		
		case _add:
			if(e1->type == conststring_e || e2 && e2->type == conststring_e)
				return;
		case _sub:
		case _mul:
		case _div:
		case _mod: {
			/* arithmetic types */							
			if(e1->type != var_e && e1->type != constnum_e) {
				printf("(line %d): Invalid use of arithmetic operator on %s ", yylineno, exprtypeToString[e1->type]);
				printUnionMember(e1);
				flagError();
			}
			if(e2 && e2->type != var_e && e2->type != constnum_e) {
				printf("(line %d): Invalid use of arithmetic operator on %s ", yylineno, exprtypeToString[e2->type]);
				printUnionMember(e2);
				flagError();
			}
			return;
		}
		
		case _uminus: assert(0 && "uminus");

		case _and:	
		case _or:	
		case _not:	{
						/* booleans types or convertible to boolean(BIG KAPPA) */
						return;
					}

		case _if_eq:
		case _if_noteq:	
						{
							/* expressions have to be of same type(BIG KAPPA) */
							return;				
						}	

		case _if_lessereq:
		case _if_greatereq:
		case _if_less:
		case _if_greater: {
			/* arithmetic types */
			if(e1->type != var_e && e1->type != constnum_e /*&& e1->type != conststring_e*/) {
				printf("(line %d): Invalid use of comparison operator on %s ",yylineno, exprtypeToString[e1->type]);
				printUnionMember(e1);
				flagError();
			}
			if(e2 && e2->type != var_e && e2->type != constnum_e /*&& e2->type != conststring_e*/) {
				printf("(line %d): Invalid use of comparison operator on %s ",yylineno, exprtypeToString[e2->type]);
				printUnionMember(e2);
				flagError();
			}
			return;
		}

		default: assert(0);
	}
}

/* returns NULL if r-value is not const */
Expr* computeConstOpConst(Expr* e1, Expr* e2, iopcode op){
	assert(e1);

	if(e1->type != constnum_e || (e2 && e2->type != constnum_e))
		return NULL;

	double arithmetic_result;
	switch(op) {

		case _add: 	
					assert(e2);							
					arithmetic_result = e1->value.numConst + e2->value.numConst;
					return new_const_expr(constnum_e, &arithmetic_result);
		case _sub:
					assert(e2);						
					arithmetic_result = e1->value.numConst - e2->value.numConst;
					return new_const_expr(constnum_e, &arithmetic_result);
		case _mul:
					assert(e2);
					arithmetic_result = e1->value.numConst * e2->value.numConst;
					return new_const_expr(constnum_e, &arithmetic_result);
		case _div:
					assert(e2);
					arithmetic_result = e1->value.numConst / e2->value.numConst;
					return new_const_expr(constnum_e, &arithmetic_result);
		case _mod:
					assert(e2);
					arithmetic_result = (int)e1->value.numConst % (int)e2->value.numConst;
					return new_const_expr(constnum_e, &arithmetic_result);
		
		case _uminus:
						assert(0 && "uminus");

		default:;
	}
	return NULL;
}

void updateAnonymVarC(Expr* e1, Expr* e2){ // u do dis?
	assert(e1);

	if (isTempExpr(e1)){
		anonymVarCounter--;
#ifdef __OPT // the fuk?
		return;
#endif
	}
	if (e2 && isTempExpr(e2))
		anonymVarCounter--;
}

double strToDouble(char* str){
	assert(str);
	double ret;
	sscanf(str, "%lf", &ret);
	return ret;
}

/*
	enum -> type, 
	constnum_e -> double, 
	constbool_e -> unsigned char, 
	conststring_e -> char * (strdup)
*/
Expr* new_const_expr(Expr_type type, void* val){
	
	Expr* expr = malloc(sizeof(Expr));
	QuadArray_Garbage_add(quads, expr);
	expr->type = type;
	expr->union_i = 0;
	switch(type){
		case constnum_e:
			expr->value.numConst = *(double *)val;
			expr->union_i = 1;
			break;
		case constbool_e:
			expr->value.boolConst = *(unsigned char *)val;
			expr->union_i = 3;
			break;
		case conststring_e:
			expr->value.strConst = strdup((char *)val);
			expr->union_i = 2;
			break;
		case nil_e:
			expr->union_i = 4;
			break;
		default:
			assert(0);
	}
	return expr;
}

Expr* new_lvalue_expr(SymbolTableEntry * entry){
	if(entry == NULL)
		return NULL;

	Expr* expr = malloc(sizeof(Expr));
	QuadArray_Garbage_add(quads, expr);
	expr->value.sym = entry;
	expr->union_i = 0;
	switch(entry->type){
		case USERFUNC:
			expr->type = programfunc_e; break;
		case LIBFUNC:
			expr->type = libfunc_e;	break;
		default:
			expr->type = var_e;
	}
	return expr;
}


uint digitCount(uint num){
	if(num == 0)
		return 1;
	uint digits = 0;
	while(num != 0){
		num = num / 10;
		digits++;
	}
	return digits;
}

/* returns entry with anonymVarCounter ID or create, insert & return entry with anonymVarCounter ID*/
SymbolTableEntry* insertAnonymVar(){
	uint len = digitCount(anonymVarCounter);
	char* varName = malloc(len + 3);
	varName[0] = '_';
	varName[1] = 't';
	sprintf(&varName[2], "%u", anonymVarCounter);
	anonymVarCounter++;
	SymbolTableEntry* entry;
	entry = ScopeList_lookup_currScope(list, varName);
	if(entry != NULL){
		free(varName);
		return entry;
	}
	token tok;
	tok.numline = yylineno;
	tok.content = varName;
	entry = createEntry(&tok, _LOCAL);
	insertEntry(entry);
	if(functionLocalOffset == -1){
		entry->value.varVal->offset = programVarOffset++;
		entry->value.varVal->space = program_var;
	}
	else{
		entry->value.varVal->offset = functionLocalOffset++;
		entry->value.varVal->space = function_local;
	}
	free(varName);
	return entry;
}

void resetAnonymVars(){
	
	#ifdef __OPT
		static uint deadcodeIndex = 0;
		QuadArray_deadcode_elimination(quads, deadcodeIndex);
		deadcodeIndex = QuadArray_nextQuad(quads);
	#endif
	anonymVarCounter = 0;
}

token createAnonymousFunction(){ // MALLOCED CONTENT FIX ?? fixed
	static uint funcID = 0;
	uint len = digitCount(funcID);
	char *funcName = malloc(len + 2);
	funcName[0] = '$';
	sprintf(&funcName[1], "%u", funcID);
	funcID++;

	token tok;
	tok.numline = yylineno; // ??? param ???
	tok.content = funcName;
	return tok;
}

void checkRValueWrite(Expr *expr){
	if(expr == NULL)
		return;
	if(expr->type == programfunc_e || expr->type == libfunc_e){
		flagError();
		printf("(line %u): cannot change the value of \"%s\" defined at: line %u.\n", yylineno, expr->value.sym->name, expr->value.sym->line);
		printf("functions are constant their value cannot be changed.\n");
	}
}

void printRule(const char *after, const char *before){
	//printf("%s -> %s\n", before, after);
}

void printTok(token *tok){
	printf("numline=%d,  numToken=%d,  content=\"%s\",  type=\"%s\".\n", tok->numline, tok->numToken, tok->content, tok->type);
}


SymbolTableEntry * referenceGlobal(token *tok) {
	SymbolTableEntry *entry = ScopeList_lookup_globalScope(list, tok->content);
	if(entry == NULL){
		printf("(line %u): \"%s\" undeclared.\n", tok->numline, tok->content);
		flagError();
		return NULL;
	}
	return entry;
}

SymbolTableEntry * createLocal(token *tok) {
	SymbolTableEntry *entry = ScopeList_lookup_globalScope(list, tok->content);
	if(entry != NULL){
		if(entry->type == LIBFUNC){
			printf("(line %u): \"%s\" shadows library function.\n", tok->numline, tok->content);
			flagError();
			return entry;
		}
	}
	entry = ScopeList_lookup_currScope(list, tok->content);
	if(entry != NULL){
		return entry;
	}
	entry = createEntry(tok, _LOCAL);
	insertEntry(entry);
	if(functionLocalOffset == -1){
		entry->value.varVal->offset = programVarOffset++;
		entry->value.varVal->space = program_var;
	}
	else{
		entry->value.varVal->offset = functionLocalOffset++;
		entry->value.varVal->space = function_local;
	}
	return entry;
}

SymbolTableEntry * createEntry(token *tok, enum SymbolType type){
	SymbolTableEntry *entry = malloc(sizeof(SymbolTableEntry));
	if(type == USERFUNC || type == LIBFUNC){
		Function *funcVar = malloc(sizeof(Function));
		lastFunction = funcVar;
		funcVar->list = NULL;
		funcVar->argc = 0;
		entry->value.funcVal = funcVar;
	} 
	else{
		Variable *newVar = malloc(sizeof(Variable));
		entry->value.varVal = newVar;
	}
	entry->name = strdup(tok->content);
	entry->scope = currScope;
	entry->line = tok->numline;
	entry->isActive = 1;
	entry->type = type;
	return entry;
}

void increaseScope(){
	currScope++;
	if(!functionScoped)
		ScopeList_create_scope(list, currScope, insideLoop);
	else{
		ScopeList_set_function_scope(list, functionLocalOffset);
		insideLoop = 0;
		functionLocalOffset = 0;
	}
	functionScoped = 0;
}

void decreaseScope(int dontHideScope){
	assert(currScope > 0);
	currScope--;
	if (dontHideScope)
		functionScoped = 1;
	else{
		functionScoped = 0;
		HideScopeRet info = ScopeList_hide_scope(list);
		insideLoop = info.insideLoop;
		if(info.enclosingFuncLocalOffset != -2)
			functionLocalOffset = info.enclosingFuncLocalOffset;
	}
	
}

SymbolTableEntry* insertVar(token *tok){
	EntryAndAccess lookup = ScopeList_lookup_insideout(list, tok->content, currScope);
	if (lookup.access == 0) { /* found but no access*/
		printf("(line %u): cannot access \"%s\"[%s] in scope %u.\n", tok->numline, tok->content, typeToString[lookup.entry->type], lookup.entry->scope);
		flagError();
		//return NULL;
	} 
	
	
	/*not found*/
	if(lookup.access == 3){
		enum SymbolType type = (currScope == 0) ? GLOBAL : _LOCAL;
		SymbolTableEntry *entry = createEntry(tok,type);
		insertEntry(entry);
		lookup.entry = entry;

		if(functionLocalOffset == -1){
			lookup.entry->value.varVal->offset = programVarOffset++;
			lookup.entry->value.varVal->space = program_var;
		}
		else{
			lookup.entry->value.varVal->offset = functionLocalOffset++;
			lookup.entry->value.varVal->space = function_local;
		}
	}
	


	return lookup.entry;
}


void insertArg(token *tok){
	if(nameIsLibTaken(tok->content)){
		flagError();
		printf("(line %u): formal argument \"%s\" shadows library function.\n", tok->numline, tok->content);
		return;
	}

	if (ScopeList_lookup_currScope(list, tok->content) != NULL){
		flagError();
		printf("(line %u): formal argument \"%s\" redeclared.\n", tok->numline, tok->content);
		return;
	}

	SymbolTableEntry *entry = createEntry(tok, FORMAL);
	insertEntry(entry);
	entry->value.varVal->offset = lastFunction->argc++;
	entry->value.varVal->space = formal_argument;

	Arg_node *newArg = malloc(sizeof(Arg_node));
	newArg->entry = entry;
	newArg->next = NULL;
	Arg_node *lastArg = lastFunction->list;


	if(lastArg == NULL){
		lastFunction->list = newArg;
		return;
	}
	while(lastArg->next != NULL)
		lastArg = lastArg->next;
	lastArg->next = newArg;
}

SymbolTableEntry* insertFunc(token *tok){
	if(nameIsLibTaken(tok->content)){
		flagError();
		printf("(line %u): \"%s\" shadows library function.\n", tok->numline, tok->content);
		//return NULL;
	}
	
	SymbolTableEntry *check = ScopeList_lookup_currScope(list, tok->content);
	if(check != NULL){
		flagError();
		printf("(line %u): redefinition of \"%s\" previous definition was here: (line %u).\n", tok->numline, tok->content, check->line);
		//return NULL;
	}

	SymbolTableEntry *entry = createEntry(tok, USERFUNC);
	insertEntry(entry);
	entry->value.funcVal->address = QuadArray_nextQuad(quads);
	return entry;
}

void flagError(void){
	errorFlag = 1;
}

void insertEntry(SymbolTableEntry* STEntry){
	assert(STEntry);
	SymTable_put(symtable, STEntry->name, STEntry);
	ScopeList_insert(list, STEntry);
}

void initializeLibFunc(char* name){
	Function* libFunc = malloc(sizeof(struct Function));
	lastFunction = libFunc;
	libFunc->list = NULL;
	libFunc->argc = 0;
	SymbolTableEntry* STEntry = malloc(sizeof(struct SymbolTableEntry));
	STEntry->name = strdup(name);
	STEntry->scope = 0;
	STEntry->line = 0;
	STEntry->isActive = 1;
	STEntry->value.funcVal = libFunc;
	STEntry->type = LIBFUNC;
	insertEntry(STEntry);
}

char* nameIsLibTaken(char *name){
	for(int i=0; i<LIB_FUNC_AMOUNT; i++){
		if(strcmp(libFuncsNames[i], name) == 0)
			return libFuncsNames[i];
	}
	return NULL;
}

int yyerror(char* yaccProvidedMessage){
	if(strcmp(yytext, "") == 0)
		fprintf(stderr, "%s: at line %d, unexpected end\n", yaccProvidedMessage, yylineno);
	else
		fprintf(stderr, "%s: at line %d, before token: %s\n", yaccProvidedMessage, yylineno, yytext );
	fprintf(stderr, "INVALID SYNTAX\n");
	flagError();
	return 0;
}


bool argv_parser(int argc, char **argv, char **outFile, bool *run, char **inFile){
	for(int i = 1; i < argc; i++){
		if(strcmp(argv[i], "-run") == 0)
			*run = 1;
		else if(strcmp(argv[i], "-o") == 0){
			if(i == argc -1){
				printf("alphac: error: missing filename after '-o'\n");
				return 0;
			}
			*outFile = argv[++i];
		}
		else{
			if(*inFile != NULL)
				printf("alphac: warning: unknown argument '%s'\n", argv[i]);
			else
				*inFile = argv[i];
		}
	}
	return 1;
}

int main(int argc, char **argv){
	char *inFile = NULL;
	char *outFile = "out.abc"; // default.
	bool run = 0; // default.
	argv_parser(argc, argv, &outFile, &run, &inFile);
	if(inFile == NULL){
		printf("alphac error: no input file\n");
		return 1;
	}
	yyin = fopen(inFile, "r");
	if(!yyin){
		perror("alphac error");
		return 1;
	}

	symtable = SymTable_new();
	list = ScopeList_new();
	quads = QuadArray_new();
	logiclist = LogicList_new();
	InstrArray instructions = NULL;
	for(int i=0; i<LIB_FUNC_AMOUNT; i++){
		initializeLibFunc(libFuncsNames[i]);
	}

	yyparse();

	if(errorFlag)
		printf("COMPILATION FAILED\n");
	else{
		#ifdef __OPT
			QuadArray_final_opt(quads);
		#endif
		//QuadArray_write_quads(quads);
		instructions = generate_target_code(quads);
	}
  	fclose(yyin);
	QuadArray_free(quads);
	ScopeList_free(list);	
	SymTable_free_content(symtable);
	SymTable_free(symtable);
	//InstrArray_print(instructions);
	errorFlag = InstrArray_serializer(instructions, outFile, programVarOffset);
	if(!errorFlag){
		InstrArray_free(instructions);
	}

	if(run == 1 && errorFlag == 0){
		char *vm_argv[3];
		vm_argv[0] = "./alpha";
		vm_argv[1] = outFile;
		vm_argv[2] = NULL;
		execvp(vm_argv[0], vm_argv);
		printf("could not run AVM.\n");
	}
	return 0;
}
