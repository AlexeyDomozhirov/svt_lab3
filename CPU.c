#include "CPU.h"

#include <stdlib.h>
#include <string.h>

CPU *cpu_init(void) {
  CPU *cpu = (CPU *)malloc(sizeof(CPU));
  memset(cpu, 0, sizeof(CPU));
  cpu->is_running = true;
  return cpu;
}
