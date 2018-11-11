#ifndef PARSER_C
#define PARSER_C

#include "stackvm++.h"

#define NO_ARG_INSTRUCTION(lowername, uppername) \
  else if(skip(&string, lowername)) {parsed->instruction = uppername;}

char labelnames[256][32];
int labelnamecount = 0;
char funcnames[256][32];
int funcnamecount = 0;
char autovarnames[256][32];
int autovarnamecount = 0;

unsigned char skip(char **str, const char *pre) {
  unsigned char found = strncmp(pre, *str, strlen(pre)) == 0;
  if(found)
    *str += strlen(pre);
  return found;
}

void skipmany(char **str, char ch) {
  while(**str == ch)
    (*str)++;
}

/*Il primo separatore trovato nella stringa originale (spazio, a capo o \0) viene sostituito da \0;
* il resto della stringa rimane invariato. La stringa ritornata si appoggia su quella originale.*/
char *readword(char **str) {
  char *word = *str;
  skipmany(&word, ' ');
  int i = 0;
  while(!(isspace(word[i]) || word[i] == '\0' || word[i] == '#' || word[i] == '(' || word[i] == ')' || word[i] == ','))
    ++i;
  word[i] = '\0';
  *str = &(word[i + 1]);
  return word;
}

int find(char arr[256][32], int length, char *string) {
  int i;
  for(i = 0; i < length; ++i) {
    if(strcmp(arr[i], string) == 0)
      return i;
  }
  return -1;
}

long estrtol(char **str, int base) {
  char *end;
  long result = strtol(*str, &end, base);
  if(end == *str) ERR("Parse error, numeric argument expected\n")
  else {
    *str = end;
    return result;
  }
}

long estrtoul(char **str, int base) {
  char *end;
  long result = strtoul(*str, &end, base);
  if(end == *str || result < 0) ERR("Parse error, unsigned numeric argument expected\n")
  else {
    *str = end;
    return result;
  }
}

Step *parseStep(char *string, unsigned int ln) {
  Step *parsed = malloc(sizeof(Step));
  parsed->next = NULL;
  skipmany(&string, ' ');
  if(*string == '#' || *string == '\n')
    return NULL;
  //printf("Parsing: %s", string);
  
  if(skip(&string, "pushfrom")) {
    parsed->instruction = PUSHFROM;
    char *argument = readword(&string);
    if((parsed->arg = find(autovarnames, autovarnamecount, argument)) == -1) {
      parsed->arg = estrtol(&argument, 10);
    }
  }
  else if(skip(&string, "popto")) {
    parsed->instruction = POPTO;
    char *argument = readword(&string);
    if((parsed->arg = find(autovarnames, autovarnamecount, argument)) == -1) {
      parsed->arg = estrtol(&argument, 10);
    }
  }
  else if(skip(&string, "push")) {
    parsed->instruction = PUSH;
    skipmany(&string, ' ');
    if(skip(&string, "'")) {
      char parsedchar = *(string++);
      if(skip(&string, "'"))
        parsed->arg = parsedchar;
      else PERR("Invalid push argument")
    }
    else
      parsed->arg = estrtol(&string, 10);
  }
  else if(skip(&string, "jz")) {
    parsed->instruction = JZ;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "jnz")) {
    parsed->instruction = JNZ;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "jmp")) {
    parsed->instruction = JMP;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "printheap")) {
    parsed->instruction = PRINTHEAP;
    parsed->arg = estrtoul(&string, 10);
  }
  else if(skip(&string, "load")) {
    parsed->instruction = LOAD;
    skipmany(&string, ' ');
    if(skip(&string, "*tos")) {
      parsed->tos_arg = 1;
    }
    else {
      parsed->tos_arg = 0;
      parsed->arg = estrtoul(&string, 10);
    }
    if(parsed->arg > 512)
      PERR("Not a valid heap address")
  }
  else if(skip(&string, "store")) {
    parsed->instruction = STORE;
    skipmany(&string, ' ');
    if(skip(&string, "*tos")) {
      parsed->tos_arg = 1;
    }
    else {
      parsed->tos_arg = 0;
      parsed->arg = estrtoul(&string, 10);
    }
    if(parsed->arg > 512)
      PERR("Not a valid heap address")
  }
  else if(skip(&string, "runf")) {
    parsed->instruction = RUNF;
    char *funcname = readword(&string);
    parsed->arg = find(funcnames, funcnamecount, funcname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid function name\n", funcname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "l:")) {
    parsed->instruction = NOP;
    char *labelname = readword(&string);
    int index = find(labelnames, labelnamecount, labelname);
    if(index != -1)
      labels[index] = parsed;
    else {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "defun")) {
    parsed->instruction = DEFUN;
    char *funcname = readword(&string);
    int index = find(funcnames, funcnamecount, funcname);
    if(index != -1) {
      functions[index] = parsed;
      skipmany(&string, ' ');
      skip(&string, "(");
      autovarnamecount = 0;
      int argnumber;
      for(argnumber = 0;
        (skip(&string, ","), skipmany(&string, ' '), skip(&string, "arg "));
        ++argnumber) {
        char *argname = readword(&string);
        if(find(autovarnames, autovarnamecount, argname) == -1)
          strcpy(autovarnames[autovarnamecount++], argname);
        else {
          fprintf(stderr, "Argument name '%s' already in use\n", argname);
          exit(1);
        }
      }
      
      if(skip(&string, "...")) {
        argnumber = -1;
      }
      
      if(argnumber == 0 && !skip(&string, ")"))
        PERR(") expected");
      parsed->arg = argnumber;
    }
    else {
      fprintf(stderr, "'%s' at line %d : not a valid function name\n", funcname, ln);
      exit(1);
    }
  }
  NO_ARG_INSTRUCTION("pop", POP)
  NO_ARG_INSTRUCTION("add", ADD)
  NO_ARG_INSTRUCTION("sub", SUB)
  NO_ARG_INSTRUCTION("mul", MUL)
  NO_ARG_INSTRUCTION("div", DIV)
  NO_ARG_INSTRUCTION("mod", MOD)
  NO_ARG_INSTRUCTION("ret", RET)
  NO_ARG_INSTRUCTION("stop", STOP)
  NO_ARG_INSTRUCTION("printc", PRINTC)
  NO_ARG_INSTRUCTION("printi", PRINTI)
  NO_ARG_INSTRUCTION("printstack", PRINTSTACK)
  NO_ARG_INSTRUCTION("dup", DUP)
  NO_ARG_INSTRUCTION("readi", READI)
  NO_ARG_INSTRUCTION("cmp", CMP)
  else if(skip(&string, "auto"))
    PERR("Can't mix code with local variables declaration")
  else
    PERR("Not a valid instruction")
  return parsed;
}

Step *parse(char* filename) {
  FILE *file;
  Step *first = NULL, *last;
  int ln = 0;
  char *newline = malloc(sizeof(char) * 256);
  char *newlinebgn = newline;
  //char *newline = newline;
  
  file = fopen(filename, "r");
  if(file == NULL) {
    printf("Can't open file '%s'\n", filename);
    exit(1);
  } 
  
  while(fgets(newline, 256, file) != NULL) {
    newlinebgn = newline;
    skipmany(&newlinebgn, ' ');
    char *next;
    if(skip(&newlinebgn, "l:")) {
      char *labelname = readword(&newlinebgn);
      if(find(labelnames, labelnamecount, labelname) == -1)
        strcpy(labelnames[labelnamecount++], labelname);
      else {
        fprintf(stderr, "Label '%s' already in use\n", labelname);
        exit(1);
      }
    }
    else if(skip(&newlinebgn, "defun")) {
      char *funcname = readword(&newlinebgn);
      if(find(funcnames, funcnamecount, funcname) == -1)
        strcpy(funcnames[funcnamecount++], funcname);
      else {
        fprintf(stderr, "Function name '%s' already in use\n", funcname);
        exit(1);
      }
    }
  }
  rewind(file);
  
  while(fgets(newline, 256, file) != NULL) {
    newlinebgn = newline;
    ++ln;
    Step *newStep = parseStep(newline, ln);
    if(newStep == NULL)
      continue;
    if(first != NULL) {
      last->next = newStep;
      last = newStep;
    }
    else {
      first = newStep;
      last = first;
    }
    if(newStep->instruction == DEFUN) {
      while(fgets(newline, 256, file) != NULL) {
        ++ln;
        newlinebgn = newline;
        if(skip(&newlinebgn, "  auto")) {
          char *autovarname = readword(&newlinebgn);
          if(find(autovarnames, autovarnamecount, autovarname) == -1)
            strcpy(autovarnames[autovarnamecount++], autovarname);
          else {
            fprintf(stderr, "Local variable name '%s' already in use\n", autovarname);
            exit(1);
          }
          last->next = malloc(sizeof(Step));
          last->next->instruction = PUSH;
          last = last->next;
        }
        else {
          newStep = parseStep(newline, ln);
          if(newStep != NULL) {
            last->next = newStep;
            last = last->next;
          }
          break;
        }
      }
    }
  }
  free(newline);
  fclose(file);
  return first;
}
#endif
