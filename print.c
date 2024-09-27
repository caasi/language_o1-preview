#include <stdio.h>
#include "print.h"

void print_indentation(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  "); // Two spaces per indent level
    }
}