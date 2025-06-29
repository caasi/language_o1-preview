# languag o1-preview

A functional language interpreter originally developed with OpenAI's o1-preview model, now maintained with Claude Code.

## Overview

This language supports basic functional programming features including arithmetic operations, functions, variables, scoping, and Algebraic Data Types (ADTs) with pattern matching.

## Features

- **Arithmetic Operations**: Addition, subtraction, multiplication, and division with decimal numbers
- **Functions**: ML-style function definitions with recursion and first-class functions
- **Variables and Scoping**: Lexical scoping with let bindings  
- **Algebraic Data Types**: Custom types with constructors and pattern matching
- **Control Flow**: if-then-else conditionals and case expressions
- **Error Handling**: Division by zero, stack overflow, and arity checking

## Getting Started

### Prerequisites

- **C Compiler**: You need a C compiler that supports the C11 standard (e.g., GCC).
- **Make**: To build the project using the provided Makefile.

### Building the Project

Clone the repository and navigate to the project directory:

```bash
git clone https://github.com/caasi/language_o1-preview.git
cd language_o1-preview
make
```

This will compile the source code and generate an executable named `lang`.

### Running the Program

You can run the program and enter expressions interactively or pipe them in:

```bash
./lang
```

Enter an expression and press `Ctrl+D` (or `Ctrl+Z` on Windows) to signal the end of input.

Alternatively, you can echo an expression into the program:

```bash
echo "1 + 2 * (3 + 4)" | ./lang
```


