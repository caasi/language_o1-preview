# languag o1-preview

In this project, I am going to create a small language with the help of OpenAI's
o1-preview model. My goal is to change as little as possible and let the
language model implement the language.

## Overview

This language is being developed step by step, starting from a simple calculator
that can perform basic arithmetic operations with decimal numbers, and gradually
adding more complex features such as variables, functions, and scoping rules.

## Features

- **Arithmetic Operations**: Supports addition, subtraction, multiplication, and division with decimal numbers.
- **Parentheses**: Allows grouping expressions using parentheses.
- **Abstract Syntax Tree (AST)**: Parses expressions into an AST for evaluation.
- **Lexer and Parser**: Implements a lexer to tokenize the input and a parser to build the AST.
- **Function Definitions**: Supports defining functions in the style of Standard ML (SML).
- **Function Applications**: Allows calling functions with arguments.
- **Variables and Scoping**: Introduces variable bindings with lexical scoping.
- **First-Class Functions**: Functions are treated as values that can be passed around.

## Problems

### Pros

- I can create a prototype language in one afternoon.

### Cons

- I think less about the implementation.
- The implementation is quiet naive.
    - Parser is not modular enough.
    - It's easy to forget fixing related functions when adding a new case.
    - Binary operators are not implemented as infix functions.

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

