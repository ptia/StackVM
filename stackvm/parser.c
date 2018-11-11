#ifndef PARSER_C
#define PARSER_C

#include "stackvm.h"

char labelnames[256][32];
int labelnamecount = 0;

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
  while(! (isspace(word[i]) || word[i] == '\0')) {
    ++word;
    ++i;
  }
  word[i] = '\0';
  *str = &(word[i + 1]);
  return word;
}

int find(char arr[256][32], int length, char *string) {
  int i;
  for(i = 0; i < length; ++i) {
    //printf("testing %s against %s at pos %d\n", string, arr[i], i);
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
  if(end == *str) ERR("Parse error, numeric argument expected\n")
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
  
  if(skip(&string, "push")) {
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
  else if(skip(&string, "pop")) {
    parsed->instruction = POP;
  }
  else if(skip(&string, "add")) {
    parsed->instruction = ADD;
  }
  else if(skip(&string, "sub")) {
    parsed->instruction = SUB;
  }
  else if(skip(&string, "mul")) {
    parsed->instruction = MUL;
  }
  else if(skip(&string, "div")) {
    parsed->instruction = DIV;
  }
  else if(skip(&string, "mod")) {
    parsed->instruction = MOD;
  }
  else if(skip(&string, "jz")) {
    parsed->instruction = JZ;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount+1, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "jnz")) {
    parsed->instruction = JNZ;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount+1, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "jmp")) {
    parsed->instruction = JMP;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount+1, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "printc")) {
    parsed->instruction = PRINTC;
  }
  else if(skip(&string, "print")) {
    parsed->instruction = PRINT;
  }
  else if(skip(&string, "dup")) {
    parsed->instruction = DUP;
  }
  else if(skip(&string, "load")) {
    parsed->instruction = LOAD;
    parsed->arg = estrtoul(&string, 10);
    if(parsed->arg > 256)
      PERR("Not a valid register")
  }
  else if(skip(&string, "store")) {
    parsed->instruction = STORE;
    parsed->arg = estrtoul(&string, 10);
    if(parsed->arg > 256)
      PERR("Not a valid register")
  }
  else if(skip(&string, "stop")) {
    parsed->instruction = STOP;
  }
  else if(skip(&string, "runf")) {
    parsed->instruction = RUNF;
    char *labelname = readword(&string);
    parsed->arg = find(labelnames, labelnamecount+1, labelname);
    if(parsed->arg == -1) {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else if(skip(&string, "ret")) {
    parsed->instruction = RET;
  }
  else if(skip(&string, "l:") || skip(&string, "defun")) {
    parsed->instruction = NOP;
    char *labelname = readword(&string);
    int index = find(labelnames, labelnamecount+1, labelname);
    if(index != -1)
      labels[index] = parsed;
    else {
      fprintf(stderr, "'%s' at line %d : not a valid label\n", labelname, ln);
      exit(1);
    }
  }
  else
    PERR("Not a valid instruction");
  return parsed;
}

Step *parse(char* filename) {
  FILE *file;
  Step *first = NULL, *last;
  int ln = 0;
  char newlineArr[256];
  
  file = fopen(filename, "r");
  if(file == NULL) {
    printf("Can't open file '%s'\n", filename);
    exit(1);
  } 
  
  while(fgets(newlineArr, 256, file) != NULL) {
    char *newline = newlineArr;
    skipmany(&newline, ' ');
    char *next;
    if(skip(&newline, "l:") || skip(&newline, "defun")) {
      char *labelname = readword(&newline);
      if(find(labelnames, labelnamecount+1, labelname) == -1)
        strcpy(labelnames[labelnamecount++], labelname);
      else {
        fprintf(stderr, "Label '%s' already in use\n", labelname);
        exit(1);
      }
    }
  }
  rewind(file);
  
  while(first == NULL) {
    ++ln;
    fgets(newlineArr, 256, file);
    first = parseStep(newlineArr, ln);
    last = first;
  }

  while(fgets(newlineArr, 256, file) != NULL) {
    ++ln;
    Step *newStep = parseStep(newlineArr, ln);
    if(newStep == NULL)
      continue;
    last->next = newStep;
    last = newStep;
  }
  fclose(file);
  return first;
}
#endif
