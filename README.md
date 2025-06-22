# languag o1-preview

In this project, I am creating a small language with the help of AI language models. Initially developed with OpenAI's o1-preview model, Claude Code is now taking over development and maintenance.

You can check the original conversation [here](https://chatgpt.com/share/66f3e88c-c714-8003-b36e-0220effd8bad).

## Overview

This language is developed step by step, starting from a simple calculator that can perform basic arithmetic operations with decimal numbers, and gradually adding more complex features such as variables, functions, scoping rules, and Algebraic Data Types (ADTs).

## Features

- **Arithmetic Operations**: Supports addition, subtraction, multiplication, and division with decimal numbers.
- **Parentheses**: Allows grouping expressions using parentheses.
- **Abstract Syntax Tree (AST)**: Parses expressions into an AST for evaluation.
- **Lexer and Parser**: Implements a lexer to tokenize the input and a parser to build the AST.
- **Function Definitions**: Supports defining functions in the style of Standard ML (SML).
- **Function Applications**: Allows calling functions with arguments.
- **Variables and Scoping**: Introduces variable bindings with lexical scoping.
- **First-Class Functions**: Functions are treated as values that can be passed around.
- **Algebraic Data Types (ADTs)**:
    - **ADT Definitions**: Define custom types with multiple constructors.
    - **Constructor Calls**: Instantiate ADTs using their constructors.
    - **Pattern Matching**: (Planned) Deconstruct ADTs within expressions for more expressive code.

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

## GHC Core Implementation ToDos

The language is being transformed into a GHC Core language implementation:

### High Priority
- [ ] Research current language features and identify what needs to be changed for GHC Core
- [ ] Design GHC Core syntax (expressions with Var, Lit, App, Lam, Let, Case, Cast, Tick, Type, Coercion)
- [ ] Implement Core expression data types and AST nodes
- [ ] Implement Core evaluator/interpreter
- [ ] Implement let bindings (recursive and non-recursive)

### Medium Priority  
- [ ] Update lexer to handle Core syntax (lambda, case, let, type annotations)
- [ ] Update parser for GHC Core constructs
- [ ] Add support for primitive types and literals

## Migration Plan: ML-style Language → GHC Core

### Current Language Analysis

Based on analysis of 21 test files, the current language supports:

**Core Features:**
- **Primitives**: Numbers (doubles), strings, functions as first-class values
- **Operations**: Arithmetic (`+`, `-`, `*`, `/`), comparison (`==`)
- **Control Flow**: `if-then-else` conditionals
- **Bindings**: `let-in-end` expressions with lexical scoping
- **Functions**: ML-style `fun name params = body;` with recursion and higher-order functions
- **ADTs**: Sum/product types with constructors (`type T = C1 | C2 T2`)
- **Pattern Matching**: `case-of` expressions with constructor patterns
- **Error Handling**: Division by zero, stack overflow, arity checking

### Migration Strategy

#### Phase 1: Core AST Transformation
**Replace current AST with GHC Core expressions:**

```c
// Current: 21 AST node types (AST_NUMBER, AST_BINOP, AST_FUNCTION_DEF, etc.)
// Target: 10 Core expression types

typedef enum {
    CORE_VAR,      // Variables and data constructors  
    CORE_LIT,      // Primitive literals
    CORE_APP,      // Function/constructor application
    CORE_LAM,      // Lambda abstraction
    CORE_LET,      // Let bindings (rec/non-rec)
    CORE_CASE,     // Case expressions
    CORE_CAST,     // Type coercions (simplified)
    CORE_TICK,     // Source annotations (optional)
    CORE_TYPE,     // Type expressions
    CORE_COERCION  // Type equality (simplified)
} CoreExprType;
```

#### Phase 2: Syntax Transformation
**Convert ML syntax to Core syntax:**

| Current ML Syntax | Core Equivalent |
|------------------|-----------------|
| `fun f x = x + 1;` | `let f = λx. (+) x 1` |
| `f(2, 3)` | `f 2 3` (curried application) |
| `let x = 5 in x end` | `let x = 5 in x` |
| `if c then a else b` | `case c of True → a; False → b` |
| `Constructor arg` | `Constructor arg` (same, but as Var+App) |
| `case x of C y => y` | `case x of C y → y` |

#### Phase 3: Implementation Steps

1. **✅ Research Analysis** - Completed
2. **Design Core syntax** - Define Core expression grammar
3. **Implement Core AST** - Replace parser.h structures
4. **Update lexer** - Handle `λ`, `→`, type syntax
5. **Update parser** - Parse Core expressions
6. **Implement evaluator** - Core semantics
7. **Add primitives** - Core literal types
8. **Implement let bindings** - Recursive/non-recursive

#### Phase 4: Feature Mapping

**Functions:** `fun f x, y = x + y;` becomes:
```
let f = λx. λy. (+) x y
```

**ADTs:** `type Maybe = Just Number | Nothing;` becomes:
```
-- Data constructors become regular variables
-- Just :: Number → Maybe Number  
-- Nothing :: Maybe Number
```

**Pattern Matching:** Enhanced case expressions with exhaustiveness

**Let Bindings:** Support both recursive (`letrec`) and non-recursive (`let`)

### Expected Benefits

1. **Simplification**: 21 AST nodes → 10 Core expressions
2. **Uniformity**: Constructors and functions unified under App
3. **Optimization Ready**: Core designed for transformations
4. **Type System**: Foundation for advanced type features
5. **Functional Purity**: Explicit lambda calculus representation

### Compatibility Notes

- **Syntax Change**: User syntax will change from ML-style to Core-style
- **Semantics Preserved**: Same evaluation behavior for equivalent programs
- **Test Migration**: All 21 tests need syntax updates but same expected outputs
- **Performance**: Should be similar or better due to Core's optimization-friendly design

