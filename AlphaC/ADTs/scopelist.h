#include "definitions.h"

typedef struct scopeList* ScopeList;
typedef struct EntryAndAccess EntryAndAccess;
typedef struct HideScopeRet HideScopeRet;

struct EntryAndAccess {
	SymbolTableEntry *entry;
	int access;
};

struct HideScopeRet{
	int enclosingFuncLocalOffset;
	int insideLoop;
};

ScopeList ScopeList_new(void);

int ScopeList_insert(ScopeList list, SymbolTableEntry *item);

HideScopeRet ScopeList_hide_scope(ScopeList list);

void ScopeList_free(ScopeList list);

void ScopeList_create_scope(ScopeList list, uint scope, int insideLoop);

SymbolTableEntry * ScopeList_lookup_currScope(ScopeList list, char *name);

SymbolTableEntry * ScopeList_lookup_globalScope(ScopeList list, char *name);

EntryAndAccess ScopeList_lookup_insideout(ScopeList list, char *name, uint currScope);

void ScopeList_set_function_scope(ScopeList list, int enclosingFuncLocalOffset);