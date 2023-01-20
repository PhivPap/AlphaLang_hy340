/* 	symtablehash.c
	
	Papapanagiotakis Phivos, csd3823, hy255 ask3
*/
#include "symtable.h"
#define HASH_MULTIPLIER 65599
#define SIZE 509

struct SymTable_S{
	struct node *Table[SIZE];
	uint numOfItems;
};

struct node{
	char *key;
	SymbolTableEntry *value;
	struct node *next;
};

/*	HASH FUNCTION: */
static uint SymTable_hash(char *pcKey){
	uint i;
	uint hashValue = 0U;
	i = 0U;
	while(pcKey[i]){
		hashValue = hashValue * HASH_MULTIPLIER + pcKey[i];
		i++;
	}
	return (hashValue % SIZE);
}


/* Creates empty SymTable_T

Returns: SymTable_T */
SymTable_T SymTable_new(void){
	int i;
	struct SymTable_S *ptr = malloc(sizeof(struct SymTable_S));
	for(i=0; i<SIZE; i++){	/* set all element of hash array to NULL */
		ptr->Table[i] = NULL;
	}
	ptr->numOfItems = 0; 
	return ptr;
}


/* Frees all memory allocated by the argument

Arg: SymTable_T*/
void SymTable_free(SymTable_T oSymTable){
	int i;
	struct node *ptr, *prevPtr;
	if(!oSymTable)
		return;
	for(i=0; i<SIZE; i++){	/* Loop all element of the array */
		if(oSymTable->Table[i]){
			ptr = oSymTable->Table[i];
			prevPtr = ptr;
			while(ptr){		/* Loop all elements of list */
				ptr = ptr->next;
				free(prevPtr->key);
				free(prevPtr);	/* Free element */
				prevPtr = ptr;
			}
		}
	}
	free(oSymTable);
}

static void free_entry(SymbolTableEntry *entry){
	free(entry->name);
	if(entry->type == USERFUNC || entry->type == LIBFUNC){
		struct arg *currArg = entry->value.funcVal->list;
		struct arg *prev;
		while(currArg){
			prev = currArg;
			currArg = currArg->next;
			free(prev);
		}
		free(entry->value.funcVal);
	}
	else{
		free(entry->value.varVal);
	}
	free(entry);
}

/* CARE!!! can only be used if all content is malloced.*/
void SymTable_free_content(SymTable_T oSymTable){
	int i;
	struct node *ptr;
	if(!oSymTable)
		return;
	for(i=0; i<SIZE; i++){	/* Loop all element of the array */
		if(oSymTable->Table[i]){
			ptr = oSymTable->Table[i];
			while(ptr){		/* Loop all elements of list */
				free_entry(ptr->value);
				ptr = ptr->next;
			}
		}
	}
}


/* Returns amount of elements currently contained

Arg: SymTable_T
Returns: amount of elements of SymTable_T */
uint SymTable_getLength(SymTable_T oSymTable){
	assert(oSymTable);
	return (oSymTable->numOfItems);
}


/* Adds element to oSymTable with key = pcKey and value = pvValue

Args: SymTable_T, const pointer to string, const void pointer
Returns: 1 */
int SymTable_put(SymTable_T oSymTable,char *pcKey, SymbolTableEntry *pvValue){
	struct node *ptr;
	char *tempString;
	assert(oSymTable);
	assert(pcKey);
	/*if(SymTable_contains(oSymTable, pcKey))
		return 0;*/
	tempString = malloc(strlen(pcKey)+1);
	strcpy(tempString, pcKey);
	ptr = malloc(sizeof(struct node));
	ptr->key = tempString;
	ptr->value = pvValue;
	ptr->next = oSymTable->Table[SymTable_hash(tempString)];
	oSymTable->Table[SymTable_hash(tempString)] = ptr;
	oSymTable->numOfItems++;
	return 1;
}


/* Removes and frees the element with pcKey

Args: SymTable_T, const pointer to string
Returns: 1 if an element was removed, else 0 */
int SymTable_remove(SymTable_T oSymTable, char *pcKey){
	struct node *ptr, *prevPtr;
	assert(oSymTable);
	assert(pcKey);
	ptr = oSymTable->Table[SymTable_hash(pcKey)];
	if(!ptr)
		return 0;
	if(strcmp(pcKey, ptr->key) == 0){
		oSymTable->Table[SymTable_hash(pcKey)] = ptr->next;
		free(ptr->key);
		free(ptr);
		oSymTable->numOfItems--;
		return 1;
	}
	prevPtr = ptr;
	ptr = ptr->next;
	while(ptr){
		if(strcmp(pcKey, ptr->key) == 0){
			prevPtr->next = ptr->next;
			free(ptr->key);
			free(ptr);
			oSymTable->numOfItems--;
			return 1;
		}
		prevPtr = ptr;
		ptr = ptr->next;
	}
	return 0;
}


/* Checks if an element with key = pcKey is contained 

Args: SymTable_T, const pointer to string
Returns: 1 if the element is contained, else 0 */
int SymTable_contains(SymTable_T oSymTable, char *pcKey){
	struct node *ptr;
	assert(oSymTable);
	assert(pcKey);
	ptr = oSymTable->Table[SymTable_hash(pcKey)];
	while(ptr){
		if(strcmp(pcKey, ptr->key) == 0)
			return 1;
		ptr = ptr->next;
	}
	return 0;
}


/* Returns the value of the element with key = pcKey

Args: SymTable_T, const pointer to string, 
Returns: void pointer to pvValue of element OR NULL if not contained */
SymbolTableEntry *SymTable_get(SymTable_T oSymTable, char *pcKey){
	struct node *ptr;
	assert(oSymTable);
	assert(pcKey);
	ptr = oSymTable->Table[SymTable_hash(pcKey)];
	while(ptr){
		if(strcmp(pcKey, ptr->key) == 0)
			return (ptr->value);
		ptr = ptr->next;
	}
	return NULL;
}


//------------------------------------------------------

void SymTable_print_all(SymTable_T oSymTable){
	struct node *ptr;
	assert(oSymTable);
	for(int i=0; i<SIZE; i++){	/* Loop all element of the array */
		if(oSymTable->Table[i]){
			ptr = oSymTable->Table[i];
			while(ptr){		/* Loop all elements of list */
				printf("%d: \"%s\" [%s] (line %d) (scope %d)\n",i, ptr->value->name, typeToString[ptr->value->type], ptr->value->line,  ptr->value->scope);					
				ptr = ptr->next;
			}
		}
	}
}

static uint SymTable_print_scope(SymTable_T oSymTable, uint scope){
	assert(oSymTable);
	uint entriesPrinted = 0;
	struct node *ptr;
	for(int i=0; i<SIZE; i++){	/* Loop all element of the array */
		if(oSymTable->Table[i]){
			ptr = oSymTable->Table[i];
			while(ptr){		/* Loop all elements of list */
				if(ptr->value->scope == scope){
					printf("\"%s\" [%s] (line %d) (scope %d)", ptr->value->name, typeToString[ptr->value->type], ptr->value->line,  ptr->value->scope);					
					if(ptr->value->type != USERFUNC && ptr->value->type != LIBFUNC) {
						printf("   offset: %u, ", ptr->value->value.varVal->offset);
						printf("scope space: %s", spaceToString[ptr->value->value.varVal->space] ); /* seg fault somewhere */
					}
					printf("\n");
					entriesPrinted++;
				}
				ptr = ptr->next;
			}
		}
	}
	return entriesPrinted;
}

void SymTable_print_by_scope(SymTable_T oSymTable){
	assert(oSymTable);
	uint counter = 0;
	uint scope = 0;
	while(counter < oSymTable->numOfItems){
		printf("\n\nScope %d  - - - - - - - - - - - - - - -\n\n", scope);
		assert(scope < 1000);
		counter += SymTable_print_scope(oSymTable, scope);
		scope++;
	}
	printf("\n");
}