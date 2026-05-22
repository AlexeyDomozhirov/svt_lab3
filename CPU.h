#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#define MEMORY_SIZE (1024 * 1024)
#define REG_COUNT 32

typedef struct {
  uint32_t regs[REG_COUNT];
  uint32_t pc;
  uint8_t memory[MEMORY_SIZE];
  bool is_running;
} CPU;

CPU *cpu_init(void);

#endif
