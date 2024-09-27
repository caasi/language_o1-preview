#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initialize the symbol table
SymbolTable *symbol_table_create()
{
    SymbolTable *table = malloc(sizeof(SymbolTable));
    if (!table)
    {
        fprintf(stderr, "Error: Memory allocation failed for symbol table\n");
        exit(EXIT_FAILURE);
    }
    table->capacity = 10; // Initial capacity
    table->count = 0;
    table->entries = malloc(sizeof(ConstructorEntry) * table->capacity);
    if (!table->entries)
    {
        fprintf(stderr, "Error: Memory allocation failed for symbol table entries\n");
        exit(EXIT_FAILURE);
    }
    return table;
}

// Add a constructor to the symbol table
void symbol_table_add(SymbolTable *table, const char *constructor_name, const char *adt_type_name)
{
    if (table->count >= table->capacity)
    {
        table->capacity *= 2;
        table->entries = realloc(table->entries, sizeof(ConstructorEntry) * table->capacity);
        if (!table->entries)
        {
            fprintf(stderr, "Error: Memory allocation failed while expanding symbol table\n");
            exit(EXIT_FAILURE);
        }
    }
    table->entries[table->count].constructor_name = strdup(constructor_name);
    table->entries[table->count].adt_type_name = strdup(adt_type_name);
    table->count++;
}

// Lookup a constructor in the symbol table
const char *symbol_table_lookup(SymbolTable *table, const char *constructor_name)
{
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->entries[i].constructor_name, constructor_name) == 0)
        {
            return table->entries[i].adt_type_name;
        }
    }
    return NULL; // Not found
}

// Free the symbol table
void symbol_table_free(SymbolTable *table)
{
    if (!table)
        return;
    for (int i = 0; i < table->count; i++)
    {
        free(table->entries[i].constructor_name);
        free(table->entries[i].adt_type_name);
    }
    free(table->entries);
    free(table);
}
