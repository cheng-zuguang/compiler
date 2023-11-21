//
// Created by 42134 on 2023/11/21.
//
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

void initTable(Table* table) {
    table->count = 0;
    table->capacity = 0;
    table->entries = NULL;
}

void freeTable(Table* table) {
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

/**
  * @brief  take keys and figures out which bucket in array should go in.
  * @param  entries: buckets, capacity: the hash table's capacity, bucket's key
  * @retval target entry.
  * @case   1. key for entry at that array ix is NULL, for finding, means no ele, for inserting, the ix is where insert.
  *         2. key for entry equal to the key we looking for, for finding, we found, for inserting, is updating.
  *         3. There are a key for entry, but with different key. Collision! start probing.
  */
static Entry* findEntry(Entry* entries, int capacity, ObjString* key) {
    uint32_t ix = key->hash % capacity;
    for (;;) {
        Entry* entry = &entries[ix];
        Entry* tombstone = NULL;

        if (entry->key == NULL) {
            if (IS_NIL(entry->value)) {
                // empty value
                // return tombstone for reusing the bucket.
                return tombstone != NULL ? tombstone : entry;
            } else {
                // found a tombstone
                if (tombstone == NULL) tombstone = entry;
            }
            // need string interning(called symbol).
        } else if (entry->key == key) {
            // found the key.
            return entry;
        }
        // liner probing
        ix = (ix + 1) % capacity;
    }
}

/**
  * @brief  allocate an array of buckets.
  * @param  table: hash table, capacity: hash table capacity
  * @retval None
  */
static void adjustCapacity(Table* table, int capacity) {
    Entry* entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++) {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }
    // we will recalculate the counts of hash table because of tombstone.
    table->count = 0;

    // for already exit hash table, we need re-insert entry in the bucket.
    for (int i = 0; i < table->capacity; i++) {
        Entry* entry = &table->entries[i];
        if (entry->key == NULL) continue;

        Entry* dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    // release the memory for the old array.
    FREE_ARRAY(Entry, table->entries, table->capacity);

    table->entries = entries;
    table->capacity = capacity;
}

/**
  * @brief  get the Value by key
  * @note   None
  * @param  table: hash table, key: string key, value: a pointer point the find value.
  * @retval if find, return true, or false.
  */
bool tableGet(Table* table, ObjString* key, Value* value) {
    if (table->count == 0) return false;

    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    *value = entry->value;
    return true;
}

// if an entry for that key is already present, the new value overwrites the old value.
bool tableSet(Table* table, ObjString* key, Value value) {
    // check the array has allocated.
    // manage load factor:  TABLE_MAX_LOAD(In real-world, ideal max load factor varies based on the hash function
    // ,collision-handling strategy and typical keysets.)
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD) {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry* entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = entry->key == NULL;
    // for the new empty entry the count will increment, instead of the tombstone.
    if (isNewKey && IS_NIL(entry->value)) table->count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table* table, ObjString* key) {
    if (table->count == 0) return false;

    // find the entry
    Entry* entry = findEntry(table->entries, table->capacity, key);
    if (entry->key == NULL) return false;

    // place a tombstone in the entry.
    entry->key = NULL;
    entry->value = BOOL_VAL(true);
    return true;
}

/**
  * @brief  copy all.
  * @note   None
  * @param  from: origin hash table, destination hash table.
  * @retval None
  */
void tableAddAll(Table* from, Table* to) {
    for (int i = 0; i < from->capacity; i++) {
        Entry* entry = &from->entries[i];
        if (entry->key != NULL) {
            tableSet(to, entry->key, entry->value);
        }
    }
}

/**
  * @brief  find the hash of string
  * @note   None
  * @param  table: strings(symbol) tables, chars: comparison string, length: string len, hash: hash code.
  * @retval target key or NULL.
  */
ObjString* tableFindString(Table* table, const char* chars, int length, uint32_t hash) {
    if (table->count == 0) return NULL;

    uint32_t ix = hash % table->capacity;
    for (;;) {
        Entry* entry = &table->entries[ix];
        if (entry->key == NULL) {
            // stop if we find non-tombstone entry.
            if (IS_NIL(entry->value)) return NULL;
        } else if (entry->key->length == length
            && entry->key->hash == hash
            && memcmp(entry->key->chars, chars, length) == 0) {
            // found it
            return entry->key;
        }

        ix = (ix + 1) % table->capacity;
    }
}
