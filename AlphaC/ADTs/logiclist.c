#include "logiclist.h"

typedef struct LogicList * LogicList;
typedef struct node * node;

struct LogicList{
	node trueList;
	node falseList;
};

struct node{
	int quadIndex;
	node next;
};

LogicList LogicList_new(){
	LogicList this =  malloc(sizeof(struct LogicList));
	this->trueList = NULL;
	this->falseList = NULL;
	return this;
}

void LogicList_insert_trueList(LogicList this, int index){
	assert(this);
	node newNode = malloc(sizeof(struct node));
	newNode->next = this->trueList;
	newNode->quadIndex = index;
	this->trueList = newNode;
}

void LogicList_insert_falseList(LogicList this, int index){
	assert(this);
	node newNode = malloc(sizeof(struct node));
	newNode->next = this->falseList;
	newNode->quadIndex = index;
	this->falseList = newNode;
}

void LogicList_invert(LogicList this){
	assert(this);
	node tmp = this->trueList;
	this->trueList = this->falseList;
	this->falseList = tmp;
}

void LogicList_free(LogicList this){
	assert(this);
	LogicList_free_content(this);
	free(this);
}

void LogicList_free_content(LogicList this){
	assert(this);
	node prev;
	node list = this->trueList;
	while(list){
		prev = list;
		list = list->next;
		free(prev);
	}
	list = this->falseList;
	while(list){
		prev = list;
		list = list->next;
		free(prev);
	}
	this->trueList = NULL;
	this->falseList = NULL;
}

void LogicList_backPatch(LogicList this, QuadArray quads, int truePatch, int falsePatch){
	assert(this);
	assert(quads);
	node trList = this->trueList;
	node falList = this->falseList;

	while(trList){
		QuadArray_patchLabel(quads, trList->quadIndex, truePatch);
		trList = trList->next;
	}
	while(falList){
		QuadArray_patchLabel(quads, falList->quadIndex, falsePatch);
		falList = falList->next;
	}
	LogicList_free_content(this);
}

LogicList LogicList_merge(LogicList list1, LogicList list2){
	assert(list1);
	assert(list2);
	
	node trueList1 = list1->trueList, falseList1 = list1->falseList;
	node trueList2 = list2->trueList, falseList2 = list2->falseList;

	if(trueList2 == NULL){
		list2->trueList = trueList1;
	}
	else{
		while(trueList2->next != NULL)
			trueList2 = trueList2->next;
		trueList2->next = trueList1;
	}


	if(falseList2 == NULL){
		list2->falseList = falseList1;
	}
	else{
		while(falseList2->next != NULL)
			falseList2 = falseList2->next;
		falseList2->next = falseList1;
	}
	

	free(list1);
	return list2;
}


void LogicList_print(LogicList this){
	node list = this->trueList;
	printf("======================\n");
	printf("TRUELIST = {");
	while(list){
		printf("{%d}, ", list->quadIndex);
		list = list->next;
	}
	printf("}\nFALSELIST = {");
	list = this->falseList;
	while(list){
		printf("{%d}, ", list->quadIndex);
		list = list->next;
	}
	printf("}\n======================\n");
}

void LogicList_backPatch_true(LogicList this, QuadArray quads, int truePatch){
	assert(this);
	assert(quads);
	node trList = this->trueList;
	
	while(trList){
		QuadArray_patchLabel(quads, trList->quadIndex, truePatch);
		trList = trList->next;
	}
	
	node prev;
	node list = this->trueList;
	while(list){
		prev = list;
		list = list->next;
		free(prev);
	}
	this->trueList = NULL;
}

void LogicList_backPatch_false(LogicList this, QuadArray quads, int falsePatch){
	assert(this);
	assert(quads);
	node falList = this->falseList;
	
	while(falList){
		QuadArray_patchLabel(quads, falList->quadIndex, falsePatch);
		falList = falList->next;
	}
	node prev;
	node list = this->falseList;
	while(list){
		prev = list;
		list = list->next;
		free(prev);
	}
	this->falseList = NULL;
}