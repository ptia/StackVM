#ifndef INTSTACK_C
#define INTSTACK_C

#define SIZE 256

struct Stack {
  int stack[SIZE];
  int top;
};

void init_stack(Stack *stack) {
  stack->top = -1;
}

void push(Stack *stack, int val) {
  ++stack->top;
  //printf("pushing %d to addr%d\n", val, stack->top);
  if(stack->top < SIZE)
    stack->stack[stack->top] = val;
  else {
    puts("Stack Overflow!");
    exit(1);
  }
}

int pop(Stack *stack) {
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
