/*
 * intstack
 *
 *  Created on: Nov 21, 2019
 *      Author: Chris Burke
 *      Based on:
 *      https://github.com/igniting/generic-stack/blob/master/stack.c
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "intstack.h"

void intstack_init(IntStack *s, int maxElements) {
  unsigned *storage;

  /* Try to allocate memory */
  storage = (unsigned *)malloc(sizeof(unsigned) * maxElements);
  if (storage == NULL) {
    fprintf(stderr, "Insufficient memory to initialize stack.\n");
    exit(1);
  }

  /* Initialize an empty stack */
  s->top = 0;
  s->maxElements = maxElements;
  s->storage = storage;
}

int intstack_isEmpty(IntStack *s) {
  /* top is 0 for an empty stack */
  return (s->top == 0);
}

int intstack_size(IntStack *s) {
  return s->top;
}

void intstack_push(IntStack *s, unsigned elem) {
  if (s->top == s->maxElements) {
    fprintf(stderr, "Element can not be pushed: Stack is full.\n");
    exit(1);
  }
  (s->storage)[s->top++] = elem;
}

unsigned intstack_pop(IntStack *s) {
  if (intstack_isEmpty(s)) {
    fprintf(stderr, "Can not pop from an empty stack.\n");
    exit(1);
  }
  return (s->storage)[--s->top];
}

unsigned intstack_top(IntStack *s) {
  if (intstack_isEmpty(s)) {
    fprintf(stderr, "Can not check top of an empty stack.\n");
    exit(1);
  }
  return (s->storage)[s->top-1];
}

unsigned intstack_probe(IntStack *s, int fromTop) {
  if ((fromTop<0) || (fromTop>=s->top)) {
    fprintf(stderr, "Stack probe index invalid.\n");
    exit(1);
  }
  return (s->storage)[s->top-fromTop-1];
}

void intstack_destroy(IntStack *s) {
  if (s && s->storage) {
    free(s->storage);
    s->top = 0;
  }
}
