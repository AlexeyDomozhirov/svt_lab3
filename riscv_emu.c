#include "riscv_emu.h"
#include "instr_hashtable.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static InstrHashTable *instr_table = NULL;

static InstrKey make_key(uint32_t opcode, uint32_t funct3, uint32_t funct7) {
  return (opcode << 10) | (funct3 << 7) | funct7;
}

uint32_t mem_read_u32(CPU *cpu, uint32_t addr) {
  return (uint32_t)cpu->memory[addr] | ((uint32_t)cpu->memory[addr + 1] << 8) |
         ((uint32_t)cpu->memory[addr + 2] << 16) |
         ((uint32_t)cpu->memory[addr + 3] << 24);
}

void mem_write_u32(CPU *cpu, uint32_t addr, uint32_t value) {
  cpu->memory[addr] = value & 0xFF;
  cpu->memory[addr + 1] = (value >> 8) & 0xFF;
  cpu->memory[addr + 2] = (value >> 16) & 0xFF;
  cpu->memory[addr + 3] = (value >> 24) & 0xFF;
}

void handle_add(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rd = (instr >> 7) & 0x1F;
  uint8_t rs1 = (instr >> 15) & 0x1F;
  uint8_t rs2 = (instr >> 20) & 0x1F;
  if (rd != 0)
    cpu->regs[rd] = cpu->regs[rs1] + cpu->regs[rs2];
}

void handle_sub(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rd = (instr >> 7) & 0x1F;
  uint8_t rs1 = (instr >> 15) & 0x1F;
  uint8_t rs2 = (instr >> 20) & 0x1F;
  if (rd != 0)
    cpu->regs[rd] = cpu->regs[rs1] - cpu->regs[rs2];
}

void handle_addi(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rd = (instr >> 7) & 0x1F;
  uint8_t rs1 = (instr >> 15) & 0x1F;
  int32_t imm = ((int32_t)instr) >> 20;
  if (rd != 0)
    cpu->regs[rd] = cpu->regs[rs1] + imm;
}

void handle_lw(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rd = (instr >> 7) & 0x1F;
  uint8_t rs1 = (instr >> 15) & 0x1F;
  int32_t imm = ((int32_t)instr) >> 20;
  if (rd != 0)
    cpu->regs[rd] = mem_read_u32(cpu, cpu->regs[rs1] + imm);
}

void handle_sw(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rs1 = (instr >> 15) & 0x1F;
  uint8_t rs2 = (instr >> 20) & 0x1F;
  int32_t imm = ((int32_t)instr) >> 20;
  mem_write_u32(cpu, cpu->regs[rs1] + imm, cpu->regs[rs2]);
}

void handle_beq(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rs1 = (instr >> 15) & 0x1F;
  uint8_t rs2 = (instr >> 20) & 0x1F;

  uint32_t imm = ((instr & 0xf00) >> 7) | ((instr & 0x7e000000) >> 20) |
                 ((instr & 0x80) << 4) | ((instr >> 31) << 12);

  if (imm & 0x1000)
    imm |= 0xffffe000;

  if (cpu->regs[rs1] == cpu->regs[rs2])
    cpu->pc = old_pc + imm;
}

void handle_jal(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint8_t rd = (instr >> 7) & 0x1F;

  int32_t imm = ((instr & 0x80000000) >> 11) | ((instr & 0x7fe00000) >> 20) |
                ((instr & 0x00100000) >> 9) | ((instr & 0x000ff000));

  if (imm & (1 << 20))
    imm |= ~0xFFFFF;

  if (rd != 0)
    cpu->regs[rd] = old_pc + 4;

  cpu->pc = old_pc + imm;
}

void handle_ebreak(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  cpu->is_running = false;
}

void execute(CPU *cpu, uint32_t instr, uint32_t old_pc) {
  uint32_t opcode = instr & 0x7F;
  uint32_t funct3 = 0;
  uint32_t funct7 = 0;
  switch (opcode) {
  case 0x33:
    funct3 = (instr >> 12) & 0x7;
    funct7 = (instr >> 25) & 0x7F;
    break;
  case 0x13:
  case 0x03:
  case 0x67:
    funct3 = (instr >> 12) & 0x7;
    break;
  case 0x23:
  case 0x63:
    funct3 = (instr >> 12) & 0x7;
    break;
  default:
    break;
  }
  InstrKey key = make_key(opcode, funct3, funct7);
  InstrHandler *handler_ptr = iht_get(instr_table, key);
  if (handler_ptr != NULL) {
    (*handler_ptr)(cpu, instr, old_pc);
  } else {
    cpu->is_running = false;
  }
}

void init_instr_table(void) {
  instr_table = iht_new(64);
  iht_insert(instr_table, make_key(0x33, 0, 0x00), handle_add);    // ADD
  iht_insert(instr_table, make_key(0x33, 0, 0x20), handle_sub);    // SUB
  iht_insert(instr_table, make_key(0x13, 0, 0x00), handle_addi);   // ADDI
  iht_insert(instr_table, make_key(0x03, 2, 0x00), handle_lw);     // LW
  iht_insert(instr_table, make_key(0x23, 2, 0x00), handle_sw);     // SW
  iht_insert(instr_table, make_key(0x63, 0, 0x00), handle_beq);    // BEQ
  iht_insert(instr_table, make_key(0x6F, 0, 0x00), handle_jal);    // JAL
  iht_insert(instr_table, make_key(0x73, 0, 0x00), handle_ebreak); // EBREAK
}

void run_emulator(CPU *cpu) {
  init_instr_table();
  while (cpu->is_running) {
    uint32_t instr = mem_read_u32(cpu, cpu->pc);
    uint32_t old_pc = cpu->pc;
    cpu->pc += 4;
    execute(cpu, instr, old_pc);
  }
}
