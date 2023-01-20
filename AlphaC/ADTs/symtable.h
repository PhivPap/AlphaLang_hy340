#include "definitions.h"

typedef struct SymTable_S *SymTable_T;

/* Creates empty SymTable_T

Returns: SymTable_T */
SymTable_T SymTable_new(void);


/* Frees all memory allocated by the argument

Arg: SymTable_T*/
void SymTable_free(SymTable_T oSymTable);

/* CARE!!! */
void SymTable_free_content(SymTable_T oSymTable);

/* Returns amount of elements currently contained

Arg: SymTable_T
Returns: amount of elements of SymTable_T 
Rutime Error: oSymTable = NULL */
uint SymTable_getLength(SymTable_T oSymTable);


/* Adds element to oSymTable with key = pcKey and value = pvValue

Args: SymTable_T, const pointer to string, const void pointer
Returns: 1 if element with pcKey doesnt exist, else 0 
Rutime Error: oSymTable,pcKey = NULL */
int SymTable_put(SymTable_T oSymTable,char *pcKey, SymbolTableEntry *pvValue);


/* Removes and frees the element with pcKey

Args: SymTable_T, const pointer to string
Returns: 1 if an element was removed, else 0 
Rutime Error: oSymTable,pcKey = NULL */
int SymTable_remove(SymTable_T oSymTable, char *pcKey);


/* Checks if an element with key = pcKey is contained 

Args: SymTable_T, const pointer to string
Returns: 1 if the element is contained, else 0 
Rutime Error: oSymTable,pcKey = NULL */
int SymTable_contains(SymTable_T oSymTable, char *pcKey);


/* Returns the value of the element with key = pcKey

Args: SymTable_T, const pointer to string, 
Returns: void pointer to pvValue of element OR NULL if not contained 
Rutime Error: oSymTable,pcKey = NULL */
SymbolTableEntry *SymTable_get(SymTable_T oSymTable, char *pcKey);


//-------------------------CS340--------------------------------------

void SymTable_print_all(SymTable_T oSymTable);


void SymTable_print_by_scope(SymTable_T oSymTable);