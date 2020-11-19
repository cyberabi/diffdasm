/*
 * intstack.h
 *
 *  Created on: Nov 21, 2019
 *      Author: Chris Burke
 *      Based on:
 *      https://github.com/igniting/generic-stack/blob/master/stack.h
 */

#ifndef INTSTACK_H_
#define INTSTACK_H_

typedef struct IntStack {
  int top;
  unsigned *storage;
  int maxElements;
} IntStack;

/* Function for initializing the Stack */
void intstack_init(IntStack *s, int maxElements);

/* Function for checking whether the stack is empty */
/* Returns non-zero value if stack is not empty */
int intstack_isEmpty(IntStack *s);

/* Function for returning number of elements in the stack */
int intstack_size(IntStack *s);

/* Inserts an element at top of stack */
void intstack_push(IntStack *s, unsigned elem);

/* Removes an element from top of stack */
/* Returns the element removed */
unsigned intstack_pop(IntStack *s);

/* Returns the top element without removing it from stack */
unsigned intstack_top(IntStack *s);

/* Returns the element at a specified depth without removing it from stack */
unsigned intstack_probe(IntStack *s, int fromTop);

/* Deallocates the memory allocated to stack */
void intstack_destroy(IntStack *s);

#endif /* INTSTACK_H_ */
