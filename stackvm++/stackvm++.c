#include "stackvm++.h"

int toabsaddr(int addr) {
  int absaddr = funcstackstack.stack[funcstackstack.top] + addr + 1;
  if(absaddr < stack.top + 1) {
    return absaddr;
  }
  else ERR("Can't push from / pop to addr %d: it's beyond the current TOS\n", addr)
}

Step *exec_step(Step *step) {
  int tos;
  int absaddr;
  int readint;
  Step *next = step->next;
  //printf("executing instruction %d with argument %d\n", step->instruction, step->arg);
  switch(step->instruction) {
    case PUSH:
      push(&stack, step->arg);
      break;
    case POP:
      pop(&stack);
      break;
    case ADD:
      push(&stack, pop(&stack) + pop(&stack));
      break;
    case SUB:
      tos = pop(&stack);
      push(&stack, pop(&stack) - tos);
      break;
    case MUL:
      tos = pop(&stack);
      push(&stack, pop(&stack) * tos);
      break;
    case DIV:
      tos = pop(&stack);
      push(&stack, pop(&stack) / tos);
      break;
    case MOD:
      tos = pop(&stack);
      push(&stack, pop(&stack) % tos);
      break;
    case CMP:
      tos = pop(&stack);
      int second = pop(&stack);
      if(second > tos)
        push(&stack, 1);
      else if(second == tos)
        push(&stack, 0);
      else if(second < tos)
        push(&stack, -1);
      break;
    case JZ:
      if(pop(&stack) == 0) next = labels[step->arg];
      break;
    case JNZ:
      if(pop(&stack) != 0) next = labels[step->arg];
      break;
    case JMP:
      next = labels[step->arg];
      break;
    case PRINTI:
      printf("%d\n", pop(&stack));
      break;
    case READI:
      scanf("%d", &readint);
      push(&stack, readint);
      break;
    case PRINTC:
      printf("%c", pop(&stack));
      break;
    case PRINTSTACK:
      printf("Stack: ");
      print_stack(&stack);
      putchar('\n');
      break;
    case PRINTHEAP:
      tos = pop(&stack);
      printf("Heap: ");
      int i;
      for(i = 0; i < step->arg; ++i)
        printf("#%d = %d; ", tos+i, heap[tos+i]);
      putchar('\n');
      break;
    case DUP:
      tos = pop(&stack);
      push(&stack, tos);
      push(&stack, tos);
      break;
    case LOAD:
      push(&stack, heap[step->tos_arg ? pop(&stack) : step->arg]);
      break;
    case STORE:
      heap[step->tos_arg ? pop(&stack) : step->arg] = pop(&stack);
      break;
    case RUNF:
      push_stepstack(&retstack, step->next);
      next = functions[step->arg];
      stack.min = next->arg > -1 ? stack.top - next->arg : funcstackstack.stack[funcstackstack.top];
      push(&funcstackstack, stack.min);
      if(stack.min < -1)
        ERR("Called a function that requires too many parameters, the stack isn't high enough.\n")
      break;
    case RET:
      pop(&funcstackstack);
      stack.min = funcstackstack.stack[funcstackstack.top];
      next = pop_stepstack(&retstack);
      break;
    case PUSHFROM:
      if(step->arg >= 0)
        push(&stack, stack.stack[toabsaddr(step->arg)]);
      else if(stack.top + step->arg > stack.min)
        push(&stack, stack.stack[stack.top + step->arg]);
      else
        ERR("Trying to push from a cell beyond the current scope!\n");
      break;
    case POPTO:
      if(step->arg >= 0)
        stack.stack[toabsaddr(step->arg)] = pop(&stack);
      else if(stack.top + step->arg > stack.min)
        stack.stack[stack.top + step->arg] = pop(&stack);
      else
        ERR("Trying to pop to a cell beyond the current scope!\n");
      break;
    case STOP:
      next = NULL;
      break;
    default:
      break;
  }
  return next;
}

void freeProgram(Step *first) {
  Step *curr = first;
  while(curr != NULL) {
    Step *next = curr->next;
    free(curr);
    curr = next;
  }
}

int main(int argc, char **argv) {
  char *filename;
  if(argc < 2) {
    ERR("Select file to run\n")
  }
  else if(argc == 2) {
    filename = argv[1];
  }
  
  init_stack(&stack);
  init_stack(&funcstackstack);
  push(&funcstackstack, -1);
  init_stepstack(&retstack);
  
  Step *first_step = parse(filename);
  Step *next_step = first_step;
  while(next_step != NULL)
    next_step = exec_step(next_step);
  freeProgram(first_step);
  return 0;
}
