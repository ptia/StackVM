#pragma once

#define STACKVM_H

#define ERR(msg, args...) do {fprintf(stderr, msg, ## args); exit(1);} while(0);
#define PERR(msg) do {fprintf(stderr, "Parse error in row %d: %s\n", ln, msg); exit(1);} while(0);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOP 0
#define PUSH 10
#define POP 11
#define ADD 20
#define SUB 21
#define MUL 22
#define DIV 23
#define MOD 24
#define JZ 30
#define JNZ 31
#define JMP 32
#define PRINT 40
#define PRINTC 41
#define DUP 50
#define LOAD 60
#define STORE 61
#define STOP 70
#define RUNF 80
#define RET 81

typedef struct Stack Stack;
typedef struct Step Step;
typedef struct StepStack StepStack;
struct Step {
  unsigned char instruction;
  int arg;
  Step *next;
};

Step *labels[256];
Stack stack;
int reg[256];
StepStack retstack;

#include "stepstack.c"
#include "intstack.c"
#include "parser.c"
