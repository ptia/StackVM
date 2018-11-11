#include "stackvm.h"

Step *exec_step(Step *step) {
  int tos;
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
    case JZ:
      if(pop(&stack) == 0) return labels[step->arg];
      else break;
    case JNZ:
      if(pop(&stack) != 0) return labels[step->arg];
      else break;
    case JMP:
      return labels[step->arg];
    case PRINT:
      printf("%d\n", pop(&stack));
      break;
    case PRINTC:
      printf("%c", pop(&stack));
      break;
    case DUP:
      push(&stack, stack.stack[stack.top]);
      break;
    case LOAD:
      //stack.stack[stack.top] = reg[step->arg];
      push(&stack, reg[step->arg]);
      break;
    case STORE:
      reg[step->arg] = pop(&stack);
      break;
    case RUNF:
      push_stepstack(&retstack, step->next);
      return labels[step->arg];
    case RET:
      return pop_stepstack(&retstack); 
    case STOP:
      return NULL;
    default:
      break;
  }
  return step->next;
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
  if(argc < 2)
    ERR("Select file to run\n")
  init_stack(&stack);
  init_stepstack(&retstack);
  Step *firstStep = parse(argv[1]);
  Step *nextStep = firstStep;
  while(nextStep != NULL)
    nextStep = exec_step(nextStep);
  freeProgram(firstStep);
  return 0;
}
