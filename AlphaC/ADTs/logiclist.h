#include "quadarray.h"
#include "definitions.h"

typedef struct LogicList * LogicList;

LogicList LogicList_new();

void LogicList_insert_trueList(LogicList this, int index);

void LogicList_insert_falseList(LogicList this, int index);

void LogicList_invert(LogicList this);

void LogicList_free_content(LogicList this);

void LogicList_free(LogicList this);

void LogicList_backPatch(LogicList this, QuadArray quads, int truePatch, int falsePatch);

LogicList LogicList_merge(LogicList list1, LogicList list2);

void LogicList_print(LogicList this);

void LogicList_backPatch_true(LogicList this, QuadArray quads, int truePatch);

void LogicList_backPatch_false(LogicList this, QuadArray quads, int truePatch);