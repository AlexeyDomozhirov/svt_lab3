#include "riscv_emu.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test(void) {
  CPU *cpu = cpu_init();

  uint32_t program[] = {
      0x00A00293, // addi x5, x0, 10
      0x02000313, // addi x6, x0, 32
      0x006283B3, // add x7, x5, x6
      0x00100073  // ebreak
  };

  memcpy(cpu->memory, program, sizeof(program));

  run_emulator(cpu);

  assert(cpu->regs[7] == 42);

  free(cpu);
}

void test_addi(void) {
  CPU *cpu = cpu_init();
  uint32_t prog[] = {
      0x00A00293, // addi x5, x0, 10
      0x00100073  // ebreak
  };
  memcpy(cpu->memory, prog, sizeof(prog));
  run_emulator(cpu);
  assert(cpu->regs[5] == 10);
  free(cpu);
}

void test_add(void) {
  CPU *cpu = cpu_init();
  uint32_t prog[] = {
      0x00A00293, // addi x5, x0, 10
      0x01400313, // addi x6, x0, 20
      0x006283B3, // add x7, x5, x6
      0x00100073  // ebreak
  };
  memcpy(cpu->memory, prog, sizeof(prog));
  run_emulator(cpu);
  assert(cpu->regs[7] == 30);
  free(cpu);
}

void test_sub(void) {
  CPU *cpu = cpu_init();
  uint32_t prog[] = {
      0x01400293, // addi x5, x0, 20
      0x00800313, // addi x6, x0, 8
      0x406283B3, // sub x7, x5, x6
      0x00100073  // ebreak
  };
  memcpy(cpu->memory, prog, sizeof(prog));
  run_emulator(cpu);
  assert(cpu->regs[7] == 12);
  free(cpu);
}

void test_fib(void) {
  CPU *cpu = cpu_init();
  uint32_t prog[] = {0x00000293, 0x00100313, 0x00a00393, 0x00038c63, 0x00628433,
                     0x00030293, 0x00040313, 0xfff38393, 0xfedff06f, 0x00100073,
                     0x00100073, 0x00100073, 0x00100073};
  memcpy(cpu->memory, prog, sizeof(prog));
  run_emulator(cpu);
  assert(cpu->regs[8] == 89);
  free(cpu);
}

int main(void) {
  test_addi();
  test_add();
  test_sub();
  test_fib();
  return 0;
}
