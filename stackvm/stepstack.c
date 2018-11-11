#ifndef STEPSTACK_C
#define STEPSTACK_C

#include "stackvm.h"

#define STEPSTACK_SIZE 256

struct StepStack {
  Step *stack[STEPSTACK_SIZE];
  int top;
};

void init_stepstack(StepStack *stack) {
  stack->top = -1;
}

void push_stepstack(StepStack *stack, Step *step) {
  ++stack->top;
  //printf("pushing %d to addr%d\n", val, stack->top);
  if(stack->top < STEPSTACK_SIZE)
    stack->stack[stack->top] = step;
  else {
    puts("Stack Overflow!");
    exit(1);
  }
}

Step *pop_stepstack(StepStack *stack) {
  //printf("popping from addr%d\n", stack->top);
  if(stack->top >= 0) {
    return stack->stack[stack->top--];
  }
  else {
    puts("Stack Underflow!");
    exit(1);
  }
}
#endif
