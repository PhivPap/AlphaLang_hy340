#include "vstack.h"

typedef unsigned int uint;
typedef struct node node;
typedef struct avm_table Atable;
typedef struct avm_memcell avm_memcell;
typedef unsigned char bool;


Atable * Atable_create();

void Atable_insert(Atable*, avm_memcell*, avm_memcell*);

avm_memcell * Atable_get_elem();

void Atable_increase_ref();

void Atable_decrease_ref();

uint Atable_get_currSize(Atable*);

char* Atable_getTable(Atable*);

Atable* Atable_copy_memberkeys(Atable* this);

Atable* Atable_copyObj(Atable* to_copy);