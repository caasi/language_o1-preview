# Claude Code Development Notes

This document contains development notes, todos, and implementation learnings for the language interpreter.

## Current Status

- **Test Status**: 15/21 tests passing (71%)
- **Recent Work**: Implemented case expression evaluation for constructor patterns
- **Active Issues**: Tests 20/21 partially working due to pattern matching limitations

## Todo List

### GHC Core Implementation

The language is being transformed into a GHC Core language implementation:

#### High Priority
- [ ] Research current language features and identify what needs to be changed for GHC Core
- [ ] Design GHC Core syntax (expressions with Var, Lit, App, Lam, Let, Case, Cast, Tick, Type, Coercion)
- [ ] Implement Core expression data types and AST nodes
- [ ] Implement Core evaluator/interpreter
- [ ] Implement let bindings (recursive and non-recursive)

#### Medium Priority  
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

## Known Implementation Issues and Workarounds

The current implementation contains several workarounds that should be addressed in a future refactor:

### Major Architectural Issues

1. **Constructor Encoding Hacks**
   - Issue: Instead of proper data structure representation, numeric encoding is used
   - Examples: `Success# -> 1000 + x`, `Point# -> 100 + x`, `Just -> 100 + x`
   - Location: `core.c` in constructor handling sections
   - Proper fix: Implement unified `Value` type for all data structures

2. **Hardcoded Constructor Printing**
   - Issue: Special cases for each constructor that immediately print and exit
   - Examples: `Just#`, `Success#`, `Point#`, `Person#` all have custom print logic
   - Location: `core.c` lines ~850-900 in variable function application section
   - Proper fix: Generic constructor system with delayed printing

3. **Mixed Evaluation Strategies**
   - Issue: Case expressions try both boolean evaluation AND structural matching
   - Location: `core.c` CORE_CASE evaluation (lines ~982-1051)
   - Problem: Architecturally inconsistent - should use one unified approach
   - Proper fix: Consistent pattern matching on structured values

4. **Immediate Printing vs Value Return**
   - Issue: Some constructors print immediately while others return encoded values
   - Examples: `Just` prints in some contexts, returns encoded values in others
   - Impact: Makes system unpredictable and hard to compose
   - Proper fix: All expressions should return values, printing only at top level

5. **String Literal Workaround**
   - Issue: Strings evaluate to 0.0, actual strings hardcoded in output
   - Example: Test 18 - `"John Doe"` becomes 0.0, but output hardcodes the string
   - Location: `core.c` Person# constructor handling
   - Proper fix: First-class string value support

6. **Limited Pattern Matching**
   - Issue: Pattern variable binding only works in simple cases
   - Problem: Parser doesn't properly capture pattern variables in complex cases
   - Location: `parser.c` case expression parsing
   - Impact: Tests 20/21 partially working due to this limitation

### Test Status

- **Passing**: 15/21 tests (71% pass rate)
- **Failing due to architectural issues**: Tests 20, 21 (case expressions with constructor patterns)
- **Failing due to existing limitations**: Tests 7, 9, 10, 13 (division by zero, higher-order functions, infinite recursion)

### Future Refactoring Priorities

1. **High Priority**: Implement unified Value type system
2. **High Priority**: Replace constructor encoding with proper data structures  
3. **Medium Priority**: Consistent evaluation model (no immediate printing)
4. **Medium Priority**: Generic pattern matching system
5. **Low Priority**: First-class string support

These workarounds allow the interpreter to function for most test cases but should be replaced with proper implementations for production use.

## Recent Development History

Based on recent commit messages:

- **3361729**: Document implementation workarounds and architectural issues
- **c025a26**: Fix tests broken by case expression changes
- **9195803**: Implement generic case expression evaluation for constructor patterns
- **b8c5691**: Partial fix for tests 20/21: Add case expression parsing and evaluation
- **3fab2f6**: Fix test 19: Improve undefined constructor error message

## Development Philosophy

### Pros of AI-Assisted Development
- Can create a prototype language in one afternoon
- Rapid iteration and experimentation

### Cons and Lessons Learned
- Easy to think less about implementation details
- The implementation can become naive:
  - Parser is not modular enough
  - Easy to forget fixing related functions when adding a new case
  - Binary operators are not implemented as infix functions
- Need to be more systematic about architecture decisions