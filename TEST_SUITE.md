# Language Interpreter Test Suite Documentation

This document describes the comprehensive test suite for our functional language interpreter, explaining the testing intentions and design rationale for each test.

## Testing Philosophy

Our test suite is designed with the following principles:

### 1. **Comprehensive Coverage**
Each test targets specific language features and edge cases to ensure robust implementation.

### 2. **Clear Testing Intentions**
Every test file contains detailed comments explaining:
- What feature is being tested
- Why this test is important
- What specific behaviors are validated
- Expected outcomes and edge cases

### 3. **Progressive Complexity**
Tests are ordered from basic functionality to complex language features.

### 4. **Error Validation**
Both successful execution and proper error handling are tested.

## Test Categories

### **Core Language Features (Tests 1-8)**

#### Test 1: Basic Lambda Functions and Currying
- **Purpose**: Validate fundamental lambda expression parsing and curried function calls
- **Tests**: `\ x . \ y . body` syntax, function application, operator functions
- **Why important**: Foundation for all functional programming features

#### Test 2: Let Bindings and Infix Operators  
- **Purpose**: Test variable binding and operator-as-function usage
- **Tests**: Let expressions, variable scoping, infix operators in function position
- **Why important**: Core binding mechanism and operator flexibility

#### Test 3: Variable Reuse and Self-Reference
- **Purpose**: Ensure variables can be used multiple times without conflicts
- **Tests**: Same variable in multiple positions, scoping consistency
- **Why important**: Prevents variable aliasing bugs and scoping errors

#### Test 4: Multiple Let Bindings and Function Composition
- **Purpose**: Test complex expressions with multiple bindings and function chaining
- **Tests**: Sequential let bindings, function composition, compact lambda syntax
- **Why important**: Real-world code patterns require multiple bindings

#### Test 5: Recursive Functions and Case Expressions
- **Purpose**: Validate recursion support and boolean pattern matching
- **Tests**: Self-referential functions, case expressions, factorial algorithm
- **Why important**: Recursion is essential for functional programming

#### Test 6: Nested Let Bindings and Variable Shadowing
- **Purpose**: Test lexical scoping with nested variable definitions
- **Tests**: Variable shadowing, scope restoration, nested expressions
- **Why important**: Prevents scoping bugs in complex expressions

#### Test 7: Division by Zero Error Handling
- **Purpose**: Validate runtime error detection and reporting
- **Tests**: Arithmetic error detection, computed zero divisor
- **Why important**: Error handling prevents crashes and provides diagnostics

#### Test 8: Constants and Simple Variable Binding
- **Purpose**: Test basic variable binding with constant values
- **Tests**: Non-function value binding, simple variable references
- **Why important**: Sanity check for basic interpreter functionality

### **Error Handling and Edge Cases (Tests 9-10, 19)**

#### Test 9: Partial Application and Arity Checking
- **Purpose**: Test function arity validation and error reporting
- **Tests**: Incomplete function application, arity error messages
- **Why important**: Prevents silent failures from incorrect function calls

#### Test 10: Infinite Recursion and Stack Overflow Detection
- **Purpose**: Validate stack overflow protection and error recovery
- **Tests**: Infinite recursive calls, stack depth limits
- **Why important**: Prevents interpreter crashes from runaway recursion

#### Test 19: Undefined Constructor Error Handling  
- **Purpose**: Test error handling for undefined symbol references
- **Tests**: Unknown constructor lookup, symbol table validation
- **Why important**: Provides clear error messages for typos and missing definitions

### **Algebraic Data Types and Constructors (Tests 11-18)**

#### Tests 11-14: Basic Constructor Applications
- **Purpose**: Test ADT constructor definition and basic usage
- **Tests**: Constructor functions, value wrapping, printing
- **Why important**: ADTs are fundamental to functional language design

#### Test 15: Nested Constructor Applications and ADT Composition
- **Purpose**: Test complex data structure creation with nested constructors
- **Tests**: Constructor nesting, ADT composition, complex value creation
- **Why important**: Real applications need nested data structures

#### Tests 16-18: Various Constructor Types
- **Purpose**: Test different constructor patterns and argument handling
- **Tests**: Different constructor types, argument passing, printing formats
- **Why important**: Ensures constructor system works across different use cases

### **Advanced Pattern Matching (Tests 20-21)**

#### Test 20: Case Expressions with Constructor Patterns (Just case)
- **Purpose**: Test pattern matching on constructed values with variable binding
- **Tests**: Constructor pattern matching, variable extraction, branch selection
- **Why important**: Pattern matching is core to functional programming

#### Test 21: Case Expressions with Constructor Patterns (Nothing case)
- **Purpose**: Test pattern matching with nullary constructors
- **Tests**: Nullary constructor matching, exhaustive patterns, default branches
- **Why important**: Complete pattern matching requires all constructor types

### **Comment System Integration (Test 22)**

#### Test 22: Comment Integration with All Language Features
- **Purpose**: Validate that GHC-style comments work correctly with all language features
- **Tests**: Single-line comments, multi-line comments, nested comments, comment placement
- **Why important**: Comments must not interfere with language semantics

## Test Execution

### Running Individual Tests
```bash
./lang tests/test1.lang    # Run specific test
```

### Running Full Test Suite
```bash
./run_tests.sh             # Run all tests and compare outputs
```

### Expected Behavior
Each test has a corresponding `.out` file with expected output. Tests validate both:
- **Successful execution** with correct results
- **Error handling** with appropriate error messages

## Test Design Rationale

### Why Each Test Matters

1. **Basic Functionality Tests (1-8)**: Ensure core language features work correctly
2. **Error Handling Tests (7, 9, 10, 19)**: Prevent crashes and provide useful diagnostics  
3. **ADT Tests (11-18)**: Validate type system and constructor functionality
4. **Pattern Matching Tests (20-21)**: Ensure case expressions work properly
5. **Comment Integration Test (22)**: Verify comments don't break language features

### Progressive Testing Strategy

Tests are designed to build upon each other:
- Start with basic expressions and bindings
- Add function definitions and applications
- Introduce recursion and case expressions
- Test error conditions and edge cases
- Validate advanced features like ADTs and pattern matching
- Ensure comment system integrates cleanly

### Coverage Goals

The test suite aims to cover:
- **All major language constructs**: lambda expressions, let bindings, case expressions, ADTs
- **Error conditions**: division by zero, undefined symbols, arity mismatches, infinite recursion
- **Edge cases**: variable shadowing, nested expressions, complex data structures
- **Integration**: comment system working with all language features

## Adding New Tests

When adding new language features, create tests that:

1. **Test the happy path**: Normal usage with expected results
2. **Test error conditions**: What happens when things go wrong
3. **Test edge cases**: Boundary conditions and unusual inputs
4. **Test integration**: How the feature works with existing functionality
5. **Document intentions**: Clear comments explaining what and why

## Test Maintenance

All tests include comprehensive comments explaining:
- Testing intention and rationale
- Expected behavior and outcomes
- What specific language features are validated
- Why this test is necessary for correctness

This documentation helps maintain the test suite as the language evolves and ensures new developers understand the testing strategy.