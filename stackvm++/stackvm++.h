#pragma once

#define ERR(msg, args...) do {fprintf(stderr, msg, ## args); exit(1);} while(0);
#define PERR(msg) do {fprintf(stderr, "Parse error in row %d: %s\n", ln, msg); exit(1);} while(0);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOP 0
#define STOP 1
#define PUSH 10
#define POP 11
#define ADD 20
#define SUB 21
#define MUL 22
#define DIV 23
#define MOD 24
#define CMP 25
#define JZ 30
#define JNZ 31
#define JMP 32
#define PRINTI 40
#define PRINTC 41
#define PRINTSTACK 42
#define PRINTHEAP 43
#define READI 44
#define DUP 50
#define LOAD 60
#define STORE 61
#define RUNF 70
#define RET 71
#define DEFUN 72
#define PUSHFROM 80
#define POPTO 81

typedef struct Stack Stack;
typedef struct Step Step;
typedef struct StepStack StepStack;
struct Step {
  unsigned char instruction;
  unsigned char tos_arg;
  int arg;
  Step *next;
};

Step *labels[256];
Step *functions[256];
Stack stack;
Stack funcstackstack;
int heap[512];
StepStack retstack;

#include "stepstack.c"
#include "intstack.c"
#include "parser++.c"
