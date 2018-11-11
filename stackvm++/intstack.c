#ifndef INTSTACK_C
#define INTSTACK_C

#define SIZE 512

struct Stack {
  int stack[SIZE];
  int top;
  int min;
};

void init_stack(Stack *stack) {
  stack->top = -1;
  stack->min = -1;
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
  if(stack->top > stack->min) {
    return stack->stack[stack->top--];
  }
  else if (stack->top >= 0) {
    printf("Relative stack underflow!, trying to pop from %d when min is %d\n", stack->top, stack->min);
    exit(1);
  }
  else {
    printf("Stack underflow!, trying to pop from %d\n", stack->top);
    exit(1);
  }
}

void print_stack(Stack *stack) {
  int i;
  for(i = 0; i <= stack->top; ++i) {
    if(stack->min == i)
      printf("min: ");
    printf("%d; ", stack->stack[i]);
  }
}
#endif
