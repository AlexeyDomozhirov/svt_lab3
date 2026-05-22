#include "instr_hashtable.h"

#include <stdlib.h>

uint32_t hash_instr_key(InstrKey h) {
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;
  return h;
}

InstrHashTable *iht_new(uint32_t size) {
  InstrHashTable *instr_hashtable = malloc(sizeof(InstrHashTable));
  if (instr_hashtable == NULL || size == 0)
    return NULL;

  uint32_t alloc_size = sizeof(Entry *) * size;
  if (size != 0 && alloc_size / sizeof(Entry *) != size)
    return NULL;

  Entry **buckets = malloc(alloc_size);
  if (buckets == NULL)
    return NULL;

  for (size_t i = 0; i < size; i++)
    buckets[i] = NULL;

  instr_hashtable->buckets = buckets;
  instr_hashtable->size = size;

  return instr_hashtable;
}

bool iht_insert(InstrHashTable *instr_hashtable, InstrKey instr_key,
                InstrHandler instr) {
  uint32_t pos = hash_instr_key(instr_key) % instr_hashtable->size;

  Entry *cur = instr_hashtable->buckets[pos];
  if (cur == NULL) {
    instr_hashtable->buckets[pos] = malloc(sizeof(Entry));
    if (instr_hashtable->buckets[pos] == NULL)
      return false;

    instr_hashtable->buckets[pos]->instr_key = instr_key;
    instr_hashtable->buckets[pos]->instr = instr;
    instr_hashtable->buckets[pos]->next = NULL;

    return true;
  }

  while (true) {
    if (cur->instr_key == instr_key) {
      return false;
    }

    if (cur->next == NULL)
      break;

    cur = cur->next;
  }
  cur->next = malloc(sizeof(Entry));
  if (cur->next == NULL)
    return false;

  cur->next->instr_key = instr_key;
  cur->next->instr = instr;
  cur->next->next = NULL;

  return true;
}

InstrHandler *iht_get(InstrHashTable *instr_hashtable, InstrKey instr_key) {
  uint32_t pos = hash_instr_key(instr_key) % instr_hashtable->size;
  Entry *cur = instr_hashtable->buckets[pos];

  while (cur != NULL) {
    if (cur->instr_key == instr_key) {
      return &cur->instr;
    }
    cur = cur->next;
  }
  return NULL;
}

void iht_free(InstrHashTable *instr_hashtable) {
  for (size_t i = 0; i < instr_hashtable->size; i++) {
    Entry *cur = instr_hashtable->buckets[i];
    while (cur != NULL) {
      Entry *next = cur->next;

      free(cur);

      cur = next;
    }
  }
  free(instr_hashtable->buckets);
}
