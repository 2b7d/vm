// @TODO(art): Load program from file

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#define VM_STACK_CAP 1024
#define VM_PROGRAM_CAP 1024

enum err {
  ERR_OK = 0,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  ERR_MEM_OUT_OF_RANGE,
  ERR_DIV_BY_ZERO
};

char *
err_to_cstr(enum err e)
{
  switch (e) {
  case ERR_STACK_OVERFLOW:
    return "ERR_STACK_OVERFLOW";

  case ERR_STACK_UNDERFLOW:
    return "ERR_STACK_UNDERFLOW";

  case ERR_MEM_OUT_OF_RANGE:
    return "ERR_MEM_OUT_OF_RANGE";

  case ERR_DIV_BY_ZERO:
    return "ERR_DIV_BY_ZERO";

  case ERR_OK:
    return "ERR_OK";

  default: assert(0 && "Unreachable");
  }
}

enum inst_kind {
  INST_KIND_PUSH,
  INST_KIND_ADD,
  INST_KIND_SUB,
  INST_KIND_MUL,
  INST_KIND_DIV,
  INST_KIND_JMP,
  INST_KIND_JMP_IF,
  INST_KIND_HALT
};

struct inst {
  enum inst_kind kind;
  int value;
};

struct vm {
  int stack[VM_STACK_CAP];
  size_t stack_size;

  struct inst program[VM_PROGRAM_CAP];
  size_t program_size;
  size_t ip;

  int halt;
};

enum err
vm_exec_inst(struct vm *vm, struct inst *inst)
{
  if ((int) vm->ip < 0 || vm->ip >= vm->program_size) {
    return ERR_MEM_OUT_OF_RANGE;
  }

  switch (inst->kind) {
  case INST_KIND_PUSH:
    if (vm->stack_size >= VM_STACK_CAP) {
      return ERR_STACK_OVERFLOW;
    }

    vm->stack[vm->stack_size++] = inst->value;
    vm->ip++;
    break;

  case INST_KIND_ADD:
    if (vm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }

    vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_SUB:
    if (vm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    vm->stack[vm->stack_size - 2] -= vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_MUL:
    if (vm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    vm->stack[vm->stack_size - 2] *= vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_DIV:
    if (vm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    if (vm->stack[vm->stack_size - 1] == 0) {
      return ERR_DIV_BY_ZERO;
    }
    vm->stack[vm->stack_size - 2] /= vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_JMP:
    vm->ip = inst->value;
    break;

  case INST_KIND_JMP_IF:
    if (vm->stack[vm->stack_size - 1] == 1) {
      vm->ip = inst->value;
    } else {
      vm->ip++;
    }
    vm->stack_size -= 1;
    break;

  case INST_KIND_HALT:
    vm->halt = 1;
    break;

  default: assert(0 && "Unreachable");
  }

  return ERR_OK;
}

void
vm_dump_stack(struct vm *vm)
{
  printf("Stack:\n");
  for (size_t i = 0; i < vm->stack_size; ++i) {
    printf("  %i\n", vm->stack[i]);
  }
}

void
vm_load_program_from_file(struct vm *vm, char *file_path)
{
  FILE *f = fopen(file_path, "r");
  if (f == NULL) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }

  while (!feof(f)) {
    char line[1000] = {0};

    size_t i = 0;
    char c = fgetc(f);

    while (c != '\n' && !feof(f)) {
      assert(i < 1000);
      line[i++] = c;
      c = fgetc(f);
    }

    if (*line == '\0') continue;

    char inst[] = {0};
    int value;

    int matches = sscanf(line, "%s %i", &inst, &value);
    assert(matches >= 1);

    if (strcmp(inst, "push") == 0) {
      assert(matches == 2);
      vm->program[vm->program_size++] = (struct inst) {
        .kind = INST_KIND_PUSH,
        .value = value
      };
    } else if (strcmp(inst, "add") == 0) {
      vm->program[vm->program_size++] = (struct inst) {
        .kind = INST_KIND_ADD,
      };
    } else if (strcmp(inst, "halt") == 0) {
      vm->program[vm->program_size++] = (struct inst) {
        .kind = INST_KIND_HALT,
      };
    } else {
      fprintf(stderr, "Unknown instruction %s\n", inst);
      exit(1);
    }
  }

  fclose(f);
}

void
vm_exec_program(struct vm *vm)
{
  while (!vm->halt) {
    enum err e = vm_exec_inst(vm, &vm->program[vm->ip]);
    vm_dump_stack(vm);
    if (e != ERR_OK) {
      fprintf(stderr, "ERROR: %s\n", err_to_cstr(e));
      exit(1);
    }
  }
}

int main(int argc, char **argv)
{
  static struct vm vm = {0};

  if (argc < 2) {
    fprintf(stderr, "Provide file\n");
    exit(1);
  }

  char *in_file = argv[1];
  vm_load_program_from_file(&vm, in_file);

  vm_exec_program(&vm);

  return 0;
}
