#ifndef INSTR_HASHTABLE_H
#define INSTR_HASHTABLE_H

#include "CPU.h"

typedef uint32_t InstrKey;

typedef void (*InstrHandler)(CPU *cpu, uint32_t addr, uint32_t old_pc);

typedef struct Entry {
  InstrKey instr_key;
  InstrHandler instr;
  struct Entry *next;
} Entry;

typedef struct InstrHashTable {
  uint32_t size;
  Entry **buckets;
} InstrHashTable;

InstrHashTable *iht_new(uint32_t size);

bool iht_insert(InstrHashTable *instr_hashtable, InstrKey instr_key,
                InstrHandler instr);

InstrHandler *iht_get(InstrHashTable *instr_hashtable, InstrKey instr_key);

void iht_free(InstrHashTable *instr_hashtable);

#endif
