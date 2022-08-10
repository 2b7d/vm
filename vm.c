// @TODO(art): Load program from file
// @TODO(art): Proper errors

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define VM_STACK_CAP 1024
#define VM_PROGRAM_CAP 1024

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

void vm_exec_inst(struct vm *vm, struct inst *inst)
{
  assert(vm->ip >= 0 && vm->ip <= vm->program_size);

  switch (inst->kind) {
  case INST_KIND_PUSH:
    assert(vm->stack_size < VM_STACK_CAP);
    vm->stack[vm->stack_size++] = inst->value;
    vm->ip++;
    break;

  case INST_KIND_ADD:
    assert(vm->stack_size > 1);
    vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_SUB:
    assert(vm->stack_size > 1);
    vm->stack[vm->stack_size - 2] -= vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_MUL:
    assert(vm->stack_size > 1);
    vm->stack[vm->stack_size - 2] *= vm->stack[vm->stack_size - 1];
    vm->stack_size -= 1;
    vm->ip++;
    break;

  case INST_KIND_DIV:
    assert(vm->stack_size > 1);
    assert(vm->stack[vm->stack_size - 1] != 0);
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
}

void vm_dump_stack(struct vm *vm)
{
  printf("Stack:\n");
  for (size_t i = 0; i < vm->stack_size; ++i) {
    printf("  %i\n", vm->stack[i]);
  }
}

static struct inst program[] = {
  { .kind = INST_KIND_JMP, .value = 2 },
  { .kind = INST_KIND_PUSH, .value = 5 },

  { .kind = INST_KIND_PUSH, .value = 3 },
  { .kind = INST_KIND_PUSH, .value = 9 },
  { .kind = INST_KIND_MUL },

  { .kind = INST_KIND_PUSH, .value = 26 },
  { .kind = INST_KIND_SUB },

  { .kind = INST_KIND_JMP_IF, .value = 9 },
  { .kind = INST_KIND_PUSH, .value = 69 },
  { .kind = INST_KIND_PUSH, .value = 420 },

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
    vm_exec_inst(&vm, &vm.program[vm.ip]);
    vm_dump_stack(&vm);
  }

  return 0;
}
