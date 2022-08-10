// @TODO(art): Load program from file

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define VM_STACK_CAP 1024
#define VM_PROGRAM_CAP 1024

enum err {
  ERR_OK = 0,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  ERR_MEM_OUT_OF_RANGE,
  ERR_DIV_BY_ZERO,
};

char *err_to_cstr(enum err e)
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

  case ERR_OK: default: return NULL;
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
  INST_KIND_HALT,
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

enum err vm_exec_inst(struct vm *vm, struct inst *inst)
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

void vm_dump_stack(struct vm *vm)
{
  printf("Stack:\n");
  for (size_t i = 0; i < vm->stack_size; ++i) {
    printf("  %i\n", vm->stack[i]);
  }
}

static struct inst program[] = {
  { .kind = INST_KIND_PUSH, .value = 69 },
  { .kind = INST_KIND_PUSH, .value = 420 },
  { .kind = INST_KIND_ADD },
  { .kind = INST_KIND_HALT },
};

int main(void)
{
  static struct vm vm = {0};

  int sz = sizeof(program) / sizeof(program[0]);
  for (size_t i = 0; i < sz; ++i) {
    assert(i < VM_PROGRAM_CAP);
    vm.program[vm.program_size++] = program[i];
  }

  while (!vm.halt) {
    enum err e = vm_exec_inst(&vm, &vm.program[vm.ip]);
    vm_dump_stack(&vm);
    if (e != ERR_OK) {
      printf("ERROR: %s\n", err_to_cstr(e));
      exit(1);
    }
  }

  return 0;
}
