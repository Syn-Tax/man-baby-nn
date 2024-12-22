#ifndef STACK_H
#define STACK_H

#include <stdint.h>

typedef struct ListObj {
	uint32_t value;
	struct ListObj* next;
} ListObj;

int push(uint32_t obj);
int pop(uint32_t* outObj);
void emptyStack();

#endif
