#include "scopelist.h"

typedef struct node * entry;

struct scopeList{
	int insideLoop;
	int functionScope;
	int enclosingFuncLocalOffset;
	uint scope;
	entry nodeList;
	ScopeList next;
};

struct node{
	SymbolTableEntry *value;
	entry next;
};

ScopeList ScopeList_new(void){
	ScopeList list = malloc(sizeof(struct scopeList));
	list->scope = 0;
	list->nodeList = NULL;
	list->next = NULL;
	list->functionScope = 0;
	list->insideLoop = 0;
	return list;
}

int ScopeList_insert(ScopeList list, SymbolTableEntry *item){
	assert(list);
	assert(item);

	while(list->next != NULL) {
		list = list->next;
	}

	entry newEntry = malloc(sizeof(struct node));
	newEntry->value = item;
	newEntry->next = list->nodeList;
	list->nodeList = newEntry;

	return 0;
}

void ScopeList_create_scope(ScopeList list, uint scope, int insideLoop){
	assert(list);
	ScopeList newlist = malloc(sizeof(struct scopeList));
	newlist->scope = scope;
	newlist->nodeList = NULL;
	newlist->next = NULL;
	newlist->functionScope = 0;
	newlist->insideLoop = insideLoop;

	while(list->next != NULL) {
		list = list->next;
	}
	
	list->next = newlist;
}


HideScopeRet ScopeList_hide_scope(ScopeList list){
	assert(list);
	HideScopeRet ret;
	ret.enclosingFuncLocalOffset = -2;
	ScopeList first = list;
	ScopeList prev;
	while(list->next != NULL){
		prev = list;
		list = list->next;
	}
	if(first == list){
		assert(0);
	}
	
	entry currNode = list->nodeList;
	entry prevNode;

	while(currNode != NULL){
		prevNode = currNode;
		currNode->value->isActive = 0;
		currNode = currNode->next;
		free(prevNode);
	}
	prev->next = NULL;
	if(list->functionScope)
		ret.enclosingFuncLocalOffset = list->enclosingFuncLocalOffset;
	ret.insideLoop = list->insideLoop;
	free(list);
	
	return ret;
}

void ScopeList_free(ScopeList list){
	assert(list);
	entry prevNode, currNode;
	ScopeList prevList;
	while(list != NULL){
		prevList = list;

		currNode = list->nodeList;
		while(currNode != NULL){
			prevNode = currNode;
			currNode = currNode->next;
			free(prevNode);
		}
		list = list->next;
		free(prevList);
	}
}

/*return entry if found, NULL if not found*/
SymbolTableEntry *ScopeList_lookup_currScope(ScopeList list, char *name){
	assert(list);
	
	while(list->next != NULL){
		list = list->next;
	}
	
	entry currNodeList = list->nodeList;
	if(currNodeList == NULL)
		return NULL;
	while(strcmp(currNodeList->value->name, name) != 0) {
		currNodeList = currNodeList->next;
		if (currNodeList == NULL) {
			return NULL;
		}
	}
	return currNodeList->value;
}

/*return entry if found, NULL if not found*/
SymbolTableEntry * ScopeList_lookup_globalScope(ScopeList list, char *name) {
	assert(list);
	assert(name);

	entry node = list->nodeList;
	assert(node);

	while(strcmp(node->value->name, name) != 0) {
		node = node->next;
		if (node == NULL) {
			return NULL;
		}
	}
	return node->value;
}

/*return entry if found, NULL if not found*/
static SymbolTableEntry * scope_lookup(ScopeList list, char *name, uint scope){
	assert(list);
	assert(name);

	for(uint i=0; i < scope; i++){
		list = list->next;
		assert(list);
	}

	entry currNodeList = list->nodeList;
	if(currNodeList == NULL) {
		return NULL;
	}
	while(strcmp(currNodeList->value->name, name) != 0){
		currNodeList = currNodeList->next;
		if (currNodeList == NULL) {
			return NULL;
		}
	}
	return currNodeList->value;
}

static int scope_is_function(ScopeList list, uint scope){
	assert(list);
	for(uint i=0; i < scope; i++){
		list = list->next;
		assert(list);
	}
	return list->functionScope;
}

#ifdef __DEBUG
static void printLists(ScopeList list){
	assert(list);
	int scopeCount = 0;
	printf("Printing Scope Lists -----------------------\n");
	while(list != NULL){
		printf("Scope %d: ", scopeCount);
		entry ntr = list->nodeList;
		while(ntr != NULL){
			printf("'%s', ", ntr->value->name);
			ntr = ntr->next;
		}
		printf("\n");
		scopeCount++;
		list = list->next;
	}
	printf("--------------------------------------------\n");
}
#endif
/*
  return 0 found but no access
  return 1 'name' has access (found var)
  return 2 'name' has access (found func)
  return 3 not found
*/
EntryAndAccess ScopeList_lookup_insideout(ScopeList list, char *name, uint currScope){
	assert(list);
	assert(name);
	//printLists(list);
	EntryAndAccess ret;
	SymbolTableEntry *entry;
	int scope = currScope;
		//check all scopes inside-out
	while(scope >= 0){
		entry = scope_lookup(list, name, scope);
		if(entry != NULL){
			ret.entry = entry;
			break;
		}
		scope--;	
	}
	
	// not found
	if(scope == -1){
		ret.access = 3;
		return ret;
	}

	// active functions always accessible
	if(entry->type == USERFUNC || entry->type == LIBFUNC){
		ret.access = 2;
		return ret;
	}

	// global scope always accessible
	if(scope == 0){
		ret.access = 1;
		return ret;
	}

	for(uint i = scope + 1; i <= currScope; i++){
		if(scope_is_function(list, i) == 1){
			ret.access = 0;
			return ret;
		}
	}

	ret.access = 1;
	return ret;
}


void ScopeList_set_function_scope(ScopeList list, int enclosingFuncLocalOffset) {
	assert(list);
	assert(list->next);
	while(list->next != NULL){
		list = list->next;
	}
	list->functionScope = 1;
	list->enclosingFuncLocalOffset = enclosingFuncLocalOffset;
}