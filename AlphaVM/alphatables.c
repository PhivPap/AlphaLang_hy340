#include "alphatables.h"

#define F_HASH_MULTIPLIER 65559
//#define PRIME_ARRAY_SIZE 1000
#define PRIME_ARRAY_SIZE 16
//#define ATABLE_INIT_SIZE_INDEX 10
#define ATABLE_INIT_SIZE_INDEX 0
#define INITIAL_VALUE 5381
#define M 33
#define LOAD_FACTOR_REACHED ((float)this->currSize / (float)capacity) > 0.7
#define DELETED_FACTOR_REACHED (((float)this->currDel + (float)this->currSize) / (float)capacity) > 0.85
#define MAX_ROOTS 100

// old
// static uint primes[PRIME_ARRAY_SIZE] = {
// 	#include "primeNumbers.txt"
// };
 
static const uint primes[PRIME_ARRAY_SIZE] = {
  	11, 23, 53, 101, 197, 389, 683, 1259, 2417,
	4733, 9371, 18617, 37097, 74093, 148073, 296099
};

typedef struct nested_ref_node nested_ref_node;

nested_ref_node* nest_stack = NULL;


struct nested_ref_node{
	Atable* table;
	nested_ref_node* next;
};

struct node{
	bool deletedBit;
	bool taken;
	avm_memcell* key;
	avm_memcell* value;
};

struct avm_table{
	node *array;
	uint ref_counter;
	uint capacity;
	uint currSize;
	uint primeIdx;
	uint currDel;
};

Atable * Atable_create(){
	Atable * this = malloc(sizeof(struct avm_table));
	this->ref_counter = 0;
	this->primeIdx = ATABLE_INIT_SIZE_INDEX;
	this->capacity = primes[ATABLE_INIT_SIZE_INDEX];
	this->currSize = 0;
	this->currDel = 0;
	this->array = calloc(primes[ATABLE_INIT_SIZE_INDEX], sizeof(struct node));
	return this;
}

static inline void Atable_destroy(Atable* this){
	//Atable_print(this);
	//printf("Destroying table\n");
	assert(this);
	node *array = this->array;
	for(int i = 0; i < this->capacity; i++){
		if(array[i].taken == 1){
			// PROB.
			avm_memcellclear(array[i].key);
			free(array[i].key);
			avm_memcellclear(array[i].value);
			free(array[i].value);
		}
	}
	free(this->array);
	free(this);
}

static inline uint hash(uint capacity, avm_memcell* key){
	char* id;
	uint len;
	if(key->type == string_m){
		id = key->data.strVal;
		len = strlen(id);
	}
	else{
		id = (char*)key;
		len = sizeof(struct avm_memcell);
	}
	uint hashValue = 0U;
	uint i = 0U;
	while(i < len){
		hashValue = hashValue * F_HASH_MULTIPLIER + id[i];
		i++;
	}
	return hashValue % capacity;
}

Atable* Atable_copyObj(Atable* to_copy){
	assert(to_copy);

	/* if to_copy array is empty return a new table instead */
	if(to_copy->currSize == 0)
		return Atable_create();

	Atable* this = malloc(sizeof(struct avm_table));
	this->ref_counter = 0;
	this->primeIdx = to_copy->primeIdx;
	this->capacity = primes[this->primeIdx];
	this->currSize = to_copy->currSize;
	this->currDel = 0; /* no need to move any deleted */
	this->array = calloc(this->capacity, sizeof(struct node));

	avm_memcell* nk = NULL;
	avm_memcell* nv = NULL;
	
	uint transferred_c = 0;
	uint transfer_max = this->currSize;
	uint cap = this->capacity;
	uint i = 0;
	uint hashIndex = 0;
	node* tc = to_copy->array;
	node* nar = this->array;
	
	while(1){
		if(tc[i].taken == 0){
			i++;
			continue;
		}
		
		nk = malloc(sizeof(struct avm_memcell));
		nv = malloc(sizeof(struct avm_memcell));
		
		/* key */
		if(tc[i].key->type == string_m){
			nk->type = string_m;
			nk->data.strVal = strdup(tc[i].key->data.strVal);
		}
		else {
			if(tc[i].key->type == table_m){
				Atable_increase_ref(tc[i].key->data.tableVal);
			}
			memcpy(nk, tc[i].key, sizeof(struct avm_memcell));
		}

		/* value */
		if(tc[i].value->type == string_m){
			nv->type = string_m;
			nv->data.strVal = strdup(tc[i].value->data.strVal);
		}
		else {
			if(tc[i].value->type == table_m){
				Atable_increase_ref(tc[i].value->data.tableVal);
			}
			memcpy(nv, tc[i].value, sizeof(struct avm_memcell));
		}

		hashIndex = hash(cap, nk);
		while(nar[hashIndex].taken != 0){
			hashIndex++;
			hashIndex %= cap;
		}
		nar[hashIndex].key = nk;
		nar[hashIndex].value = nv;
		nar[hashIndex].taken = 1;
		i++;
		if(++transferred_c == transfer_max)
			break;
	}

	assert(this);
	return this;
}

Atable* Atable_copy_memberkeys(Atable* to_copy){
	assert(to_copy);

	/* if to_copy array is empty return a new table instead */
	if(to_copy->currSize == 0)
		return Atable_create();

	Atable* this = malloc(sizeof(struct avm_table));
	this->ref_counter = 0;
	this->primeIdx = to_copy->primeIdx;
	this->capacity = primes[this->primeIdx];
	this->currSize = to_copy->currSize;
	this->currDel = 0; /* no need to move any deleted */
	this->array = calloc(this->capacity, sizeof(struct node));

	/* now to copy array keys to an integer indexed new array */
	/* new value-key pair memcells */
	avm_memcell* nk = NULL;
	avm_memcell* nv = NULL;
	double ikey = 0; /* indexed key counter */
	
	uint transferred_c = 0;
	uint transfer_max = this->currSize;
	uint cap = this->capacity;
	uint i = 0;
	uint hashIndex = 0;
	node* tc = to_copy->array;
	node* nar = this->array;
	
	while(1){
		if(tc[i].taken == 0){
			i++;
			continue;
		}
		
		nk = calloc(1, sizeof(struct avm_memcell)); // fixed 2 years lat3r
		nk->type = number_m;
		nk->data.numVal = ikey;
		nv = malloc(sizeof(struct avm_memcell));

		if(tc[i].key->type == string_m){
			nv->type = string_m;
			nv->data.strVal = strdup(tc[i].key->data.strVal);
		}
		else {
			if(tc[i].key->type == table_m){
				Atable_increase_ref(tc[i].key->data.tableVal);
			}
			/* copy key of to_copy array to new's array value */
			memcpy(nv, tc[i].key, sizeof(struct avm_memcell));
		}
		/* hash integer indexed keys of new array(0,1,2..)*/
		/* kappa integer , they are doubles .. */
		hashIndex = hash(cap, nk);
		while(nar[hashIndex].taken != 0){
			hashIndex++;
			hashIndex %= cap;
		}
		/* insert keymembers of to_copy array as values in new array */
		nar[hashIndex].key = nk;
		nar[hashIndex].value = nv;
		nar[hashIndex].taken = 1;
		i++;
		ikey++;
		if(++transferred_c == transfer_max)
			break;
	}

	assert(this);
	return this;
}


static void Atable_rehash(Atable *this){
	assert(this);
	this->currDel = 0;
	node * new_array = calloc(this->capacity, sizeof(struct node));
	
	uint i = 0;
	uint newIndex;
	uint transferred_elems = 0;
	uint max_to_tranfer = this->currSize;
	node* oArray = this->array;
	while(1){
		if(transferred_elems == max_to_tranfer)
			break;
		if(oArray[i].taken == 0){
			i++;
			continue;
		}
		newIndex = hash(this->capacity, oArray[i].key);
		while(new_array[newIndex].taken != 0){
			newIndex++;
			newIndex %= this->capacity;
		}
		new_array[newIndex].key = oArray[i].key;
		new_array[newIndex].value = oArray[i].value;
		new_array[newIndex].taken = 1;
		i++;
		transferred_elems++;
	}
	free(this->array);
	this->array = new_array;
///	Atable_print(this);
}

static void Atable_expand(Atable *this){
	assert(this);

	this->primeIdx++;
	if(this->primeIdx >= PRIME_ARRAY_SIZE){
		avm_error("array reached maximum size.\n");
	}
	//Atable_print(this);
	this->capacity = primes[this->primeIdx];
	this->currDel = 0;
	node * new_array = calloc(this->capacity, sizeof(struct node));
	
	//printf("Expanding...\n\n\n");
	uint i = 0;
	uint newIndex;
	uint transferred_elems = 0;
	uint max_to_tranfer = this->currSize;
	node* oArray = this->array;
	while(1){
		if(oArray[i].taken == 0){
			i++;
			continue;
		}
		newIndex = hash(this->capacity, oArray[i].key);
		while(new_array[newIndex].taken != 0){
			newIndex++;
			newIndex %= this->capacity;
		}
		new_array[newIndex].key = oArray[i].key;
		new_array[newIndex].value = oArray[i].value;
		new_array[newIndex].taken = 1;
		i++;
		/* usually should exit here for big tables */
		if(++transferred_elems == max_to_tranfer){
		   /* worst case i = oCapacity here */
			break;
		}
	}
	free(this->array);
	this->array = new_array;
}


// returns 1 if keys match else 0.
static inline bool sameKeys(avm_memcell* key1, avm_memcell* key2){
	if(key1->type != key2->type)
		return 0;
	if(key1->type == string_m){
		return !strcmp(key1->data.strVal, key2->data.strVal);
	}
	/* big oof */
	return key1->data.numVal == key2->data.numVal;
}



static inline void resetKey(avm_memcell *cell){
	switch(cell->type){
		case nil_m:
		case undef_m:
		case number_m:
		case string_m:
			return;

		case bool_m:{
			bool tmp = cell->data.boolVal;
			cell->data.numVal = 0;
			cell->data.boolVal = tmp;
			return;
		}
		case userfunc_m:{
			uint tmp = cell->data.funcVal;
			cell->data.numVal = 0;
			cell->data.funcVal = tmp;
			return;
		}

 		case libfunc_m:{
 			char* tmp = cell->data.libfuncVal;
 			cell->data.numVal = 0;
 			cell->data.libfuncVal = tmp;
 			return;
 		}

 		case table_m:{
 			Atable* tmp = cell->data.tableVal;
 			cell->data.numVal = 0;
 			cell->data.tableVal = tmp;
 			return;
 		}
 	}
}

//YOU NOOB YE, THIS IS COMPLICATED.. 
//PROB NEED 2 EXPAND IF DELETED + 
void Atable_insert(Atable* this, avm_memcell* key, avm_memcell* value){
	assert(this);
	assert(key);
	assert(value);
	if(key->type == nil_m || key->type == undef_m)
		avm_error("Invalid key type passed to array.\n"); // runtime error.
	bool deleteElem = 0;
	if(value->type == nil_m){
		deleteElem = 1;
	}
	resetKey(key);
	
	avm_memcell* new_value = NULL;
	avm_memcell* new_key = NULL;

	uint capacity = this->capacity;
	node* array = this->array;

	uint hashIndex = hash(this->capacity, key);
	uint i = hashIndex;
	//printf("hash returned: %d\n", hashIndex);

	// hash returned value saved in both hashIndex and i.
	while(array[i].taken == 1 || array[i].deletedBit == 1){
		if(array[i].deletedBit == 0){
			if(sameKeys(key, array[i].key)){
				avm_memcellclear(array[i].value);
				free(array[i].value);
				if(deleteElem){
					avm_memcellclear(array[i].key);
					free(array[i].key);
					array[i].key = NULL;
					array[i].value = NULL;
					array[i].deletedBit = 1;
					array[i].taken = 0; //? PLS
					this->currSize--; //? fuck mi
					this->currDel++;
					if(DELETED_FACTOR_REACHED)
						Atable_rehash(this);
				}
				else{
					new_value = malloc(sizeof(struct avm_memcell));
					memcpy(new_value, value, sizeof(struct avm_memcell));
					if(value->type == string_m)
						new_value->data.strVal = strdup(value->data.strVal);
					else if(value->type == table_m)
						Atable_increase_ref(value->data.tableVal);
					array[i].value = new_value;
				}
				return;
			}
		}
		i++;
		i %= capacity;
	}
	// here if key not found right?
	if(deleteElem){
		//assert(0); //ERROR???????????
		return;
	}

	while(array[hashIndex].deletedBit == 0 && array[hashIndex].taken == 1){
		hashIndex++;
		hashIndex %= capacity;
	}

	if(array[hashIndex].deletedBit == 1)
		this->currDel--;
	// here empty node found right?
	new_key = malloc(sizeof(struct avm_memcell));
	new_value = malloc(sizeof(struct avm_memcell));
	memcpy(new_key, key, sizeof(struct avm_memcell));
	if(key->type == string_m)
		new_key->data.strVal = strdup(key->data.strVal);
	else if(key->type == table_m)
		Atable_increase_ref(key->data.tableVal);
	memcpy(new_value, value, sizeof(struct avm_memcell));
	if(value->type == string_m)
		new_value->data.strVal = strdup(value->data.strVal);
	else if(value->type == table_m)
		Atable_increase_ref(value->data.tableVal);
	array[hashIndex].taken = 1;
	array[hashIndex].key = new_key;
	array[hashIndex].value = new_value;
	array[hashIndex].deletedBit = 0;
	this->currSize++;
	//printf("elem was added. D:\n");
	//Atable_print(this);
	if(LOAD_FACTOR_REACHED)
		Atable_expand(this);
	if(DELETED_FACTOR_REACHED)
		Atable_rehash(this);
}

/* maybe faster ? */
avm_memcell * Atable_get_elem(Atable* this, avm_memcell* key){
	assert(this);
	assert(key);
	if(key->type == nil_m || key->type == undef_m){
		avm_error("%s is not a valid key.\n", memcell_typeToString[key->type]);
	}
	resetKey(key);
	uint capacity = this->capacity;
	node* array = this->array;
	uint i = hash(this->capacity, key);


	while(array[i].taken == 1 || array[i].deletedBit == 1){
		if(array[i].deletedBit == 0){
			if(sameKeys(key, array[i].key)){
				return array[i].value;
			}
		}
		i++;
		i %= capacity;
	}
	return NULL; // return nil but maybe not here.
}

void Atable_increase_ref(Atable* this){
	assert(this);
	this->ref_counter++;
}

void Atable_decrease_ref(Atable* this){
	assert(this);
	this->ref_counter--;
	if(this->ref_counter == 0){
		Atable_destroy(this);
	}/* else {  possible leak? 
		push_candidate(this);
	}*/

}

uint Atable_get_ref(Atable* this){
	assert(this);
	return this->ref_counter;
}

uint Atable_get_currSize(Atable* this){
	assert(this);
	return this->currSize;
}


static inline bool nest_stack_lookup(Atable* this){
	assert(this);
	nested_ref_node* node = nest_stack;
	while(node){
		if(node->table == this)
			return 1;
		node = node->next;
	}
	return 0;
}

static inline void nest_stack_push(Atable* this){
	assert(this);
	nested_ref_node* node = malloc(sizeof(struct nested_ref_node));
	node->table = this;
	node->next = nest_stack;
	nest_stack = node;
}

static inline void nest_stack_pop(){
	nested_ref_node* node =  nest_stack;
	nest_stack = nest_stack->next;
	free(node);
}

char* Atable_getTable(Atable* this){
	assert(this);
	char* buffer;
	if(nest_stack_lookup(this)){
		printf("\nAVM WARNING: (line %u)\n>>Cannot print array with reference loop, marked item with [...]\n", currLine);
		buffer = malloc(sizeof(char) * 6);
		strcpy(buffer, "[...]");
		return buffer; 
	}
	nest_stack_push(this);

	uint buffLen = 60;
	uint buffIndex = 0; //points to last byte before /0
	uint newIndex;
	uint transferred_elems = 0;
	buffer = malloc(buffLen);
	uint size = this->currSize;
	node* array = this->array;
	uint i = 0;
	strcpy(buffer, "[ ");
	buffIndex = 2;
	while(1){
		if(transferred_elems == size)
			break;
		assert(strlen(buffer) == buffIndex);
		if(!array[i].taken){
			i++;
			continue;
		}

		char* k = avm_tostring(array[i].key);
		char* v = avm_tostring(array[i].value);
		newIndex = 9 + strlen(k) + strlen(v) + buffIndex;
		if(newIndex >= buffLen){
			buffLen = newIndex + 60;
			buffer = realloc(buffer, buffLen);
		}
		strcat(buffer, "{ ");
		strcat(buffer, k);
		free(k);
		strcat(buffer, " : ");
		strcat(buffer, v);
		free(v);

		if(transferred_elems == size - 1){
			buffIndex = newIndex - 1;
			strcat(buffer, " } ");
			break;
		}
		buffIndex = newIndex;
		
		strcat(buffer, " }, ");
		i++;
		transferred_elems++;
	}
	buffIndex++;
	strcat(buffer, "]");
	//printf("%d-%d\n", strlen(buffer), buffIndex);
	assert(strlen(buffer) == buffIndex);
	nest_stack_pop();
	return buffer;
}

