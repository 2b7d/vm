#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#define VM_STACK_CAP 1024

enum inst_kind {
  INST_KIND_PUSH,
  INST_KIND_ADD
};

struct inst {
  enum inst_kind kind;
  int value;
};

struct vm {
  int stack[VM_STACK_CAP];
  int stack_size;
};

void vm_exec_inst(struct vm *vm, struct inst *inst)
{
  switch (inst->kind) {
    case INST_KIND_PUSH:
      assert(vm->stack_size < VM_STACK_CAP);
      vm->stack[vm->stack_size++] = inst->value;
      break;

    case INST_KIND_ADD:
      assert(vm->stack_size > 1);
      vm->stack[vm->stack_size - 2] += vm->stack[vm->stack_size - 1];
      vm->stack_size -= 1;
      break;
    default:
  }


}

void vm_dump_stack(struct vm *vm)
{
  printf("Stack:\n");
  for (size_t i = 0; i < vm->stack_size; ++i) {
    printf("  %i\n", vm->stack[i]);
  }
}


int main(void)
{
  static struct vm vm = {0};

  struct inst inst_1 = { .kind = INST_KIND_PUSH, .value = 5 };
  struct inst inst_2 = { .kind = INST_KIND_PUSH, .value = 3 };
  struct inst inst_3 = { .kind = INST_KIND_ADD };

  vm_exec_inst(&vm, &inst_1);
  vm_dump_stack(&vm);
  vm_exec_inst(&vm, &inst_2);
  vm_dump_stack(&vm);
  vm_exec_inst(&vm, &inst_3);
  vm_dump_stack(&vm);
  vm_exec_inst(&vm, &inst_1);
  vm_dump_stack(&vm);
  vm_exec_inst(&vm, &inst_3);
  vm_dump_stack(&vm);
  return 0;
}
