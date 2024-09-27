#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "parser.h"

// Structure to hold constructor entries
typedef struct
{
    char *constructor_name;
    char *adt_type_name;
} ConstructorEntry;

// Symbol table structure
typedef struct
{
    ConstructorEntry *entries;
    int count;
    int capacity;
} SymbolTable;

// Function prototypes
SymbolTable *symbol_table_create();
void symbol_table_add(SymbolTable *table, const char *constructor_name, const char *adt_type_name);
const char *symbol_table_lookup(SymbolTable *table, const char *constructor_name);
void symbol_table_free(SymbolTable *table);

#endif // SYMBOL_TABLE_H
