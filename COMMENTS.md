# GHC-Style Comments Documentation

This document describes the comment syntax and implementation in our functional language interpreter.

## Design Intentions

### 1. **GHC Haskell Compatibility**
We implement the exact same comment syntax as GHC Haskell to:
- Make the language familiar to Haskell developers
- Follow established functional programming conventions
- Enable easy migration of commented Haskell code snippets

### 2. **Development Workflow Support**
Comments are designed to support common development practices:
- **Code Documentation**: Clear inline and block documentation
- **Debugging**: Easy commenting/uncommenting of code blocks
- **Prototyping**: Switching between different implementations
- **Code Review**: Inline explanations and notes

### 3. **Robust Implementation**
Our comment parsing is designed to be:
- **Complete**: Comments are entirely removed during lexical analysis
- **Safe**: No interference with string literals or other tokens
- **Nested**: Proper support for nested multi-line comments
- **Performant**: No impact on parsing or evaluation speed

## Syntax

### Single-Line Comments

```haskell
-- This is a single-line comment
let x = 5 in x end  -- Comments can follow code
-- Multiple consecutive single-line comments
-- are also supported
```

**Features:**
- Start with `--` and continue to end of line
- Can appear anywhere on a line
- Useful for brief explanations and notes

### Multi-Line Comments

```haskell
{- This is a multi-line comment
   that can span multiple lines
   and include formatting -}

let result = {- inline comment -} 42 in
  result
end
```

**Features:**
- Enclosed in `{- ... -}` delimiters
- Can span multiple lines
- Can appear inline within expressions
- Support for formatted documentation

### Nested Multi-Line Comments

```haskell
{- Outer comment
   {- Nested comment
      {- Even deeper nesting -}
      Back to second level
   -}
   Back to outer comment
-}
```

**Features:**
- Multi-line comments can be nested inside each other
- Proper nesting level tracking prevents premature termination
- Essential for commenting out code blocks that already contain comments
- Enables hierarchical documentation structure

## Use Cases

### Documentation

```haskell
{-
   Module: Basic Arithmetic
   Description: Simple calculator operations
   Author: Development Team
   
   This module provides basic arithmetic operations
   for the functional language interpreter.
-}

let calculate x y = 
  -- Add two numbers and return result
  x + y
in
  calculate 5 3  -- Expected: 8
end
```

### Debugging

```haskell
let debug_test = 10 in
  -- Original complex calculation (disabled for testing):
  -- let intermediate = debug_test * 2 + 1 in
  --   intermediate / 3
  -- end
  
  -- Simplified version for debugging:
  debug_test
end
```

### Code Alternatives

```haskell
{-
   Implementation A (simple):
   let result = 5 * 2 in result end
   
   Implementation B (complex):
   let x = 5 in
     let y = x + x in y end
   end
-}

-- Currently active implementation:
let result = 5 * 2 in result end
```

### Development Notes

```haskell
let current_version = 1.0 in
  {- TODO: Future enhancements
     - Add error handling
     - Implement type checking  
     - Add more operators
  -}
  current_version
end
```

## Implementation Details

### Lexical Analysis
- Comments are processed during tokenization
- Completely removed before parsing begins
- No comment tokens are generated
- Zero impact on AST structure

### Nesting Algorithm
For multi-line comments, we maintain a nesting counter:
1. Start with counter = 1 when `{-` is found
2. Increment counter for each additional `{-`
3. Decrement counter for each `-}`
4. Continue until counter reaches 0

### Edge Cases Handled
- Comments containing operators (`-- has - and + symbols`)
- Comments with string-like content (`{- "quotes" and 'apostrophes' -}`)
- Comments at end of file without newlines
- Empty comments (`{--}` and `-- `)
- Comments containing comment-like syntax

## Testing

Run the comprehensive test suite:

```bash
./test_comments.sh
```

Test files demonstrate various scenarios:
- `examples/comments_test.lang` - Basic functionality
- `examples/nested_comments_test.lang` - Nested comments
- `examples/comment_syntax_showcase.lang` - Syntax examples
- `examples/comment_edge_cases.lang` - Edge case testing
- `examples/comment_development_workflow.lang` - Workflow examples

## Benefits

### For Developers
- **Familiar Syntax**: Same as GHC Haskell
- **Flexible Placement**: Comments work anywhere
- **Safe Nesting**: No conflicts when commenting out commented code
- **Clean Output**: Comments don't affect AST or execution

### For Code Maintenance
- **Self-Documenting**: Code can carry its own documentation
- **Version Control Friendly**: Easy to track changes in comments
- **Debugging Support**: Quick enable/disable of code sections
- **Review Process**: Inline explanations for complex logic

## Examples

See the `examples/` directory for comprehensive examples demonstrating all comment features and common usage patterns.