#include <common.h>
#include <stack.h>

static ListObj* stack = NULL;

/* Unlike a real stack we have no stack pointer etc, we simply assume that we've attached a magical stack to the MB with (effectively) unlimited memory */

int push(uint32_t obj) {

	/* Verify we have the memory for our new entry on the stack */
	ListObj* newObj = malloc(sizeof(ListObj));
	if(!newObj) {
		error("Failed to allocate memory for the stack");
		return -1;
	}
	/* Assign our values and put it at the front of the list */
	newObj->value = obj;
	newObj->next = stack;
	stack = newObj;
	return 0;
}

int pop(uint32_t* outObj) {
	/* Ensure we're not trying to pop an empty stack */
	if(!stack) {
		error("Cannot call pop on empty stack");
		return -1;
	}
	/* Get our value, move our stack to the next object and free the now irrelivent one */
	*outObj = stack->value;
	ListObj* oldObj = stack;
	stack = stack->next;
	free(oldObj);
	return 0;
}


void emptyStack() {
	/* Free the whole stack */
	while(stack) {
		ListObj* nextObj = stack->next;
		free(stack);
		stack = nextObj;
	}
	stack = NULL;
}
