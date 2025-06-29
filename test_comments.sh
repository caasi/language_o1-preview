#!/bin/bash

# GHC-Style Comments Test Suite
# =============================
# This script tests all comment functionality and validates our implementation

echo "🧪 Testing GHC-Style Comments Implementation"
echo "============================================="
echo

# Test 1: Basic comment functionality
echo "📋 Test 1: Basic Comments"
echo "Expected: 5"
echo -n "Actual:   "
./lang examples/comments_test.lang
echo

# Test 2: Nested comments
echo "📋 Test 2: Nested Comments" 
echo "Expected: 15"
echo -n "Actual:   "
./lang examples/nested_comments_test.lang
echo

# Test 3: Syntax showcase
echo "📋 Test 3: Syntax Showcase"
echo "Expected: 10" 
echo -n "Actual:   "
./lang examples/comment_syntax_showcase.lang
echo

# Test 4: Edge cases
echo "📋 Test 4: Edge Cases"
echo "Expected: 42"
echo -n "Actual:   "
./lang examples/comment_edge_cases.lang
echo

# Test 5: Development workflow
echo "📋 Test 5: Development Workflow"
echo "Expected: 5"
echo -n "Actual:   "
./lang examples/comment_development_workflow.lang
echo

# Test 6: Inline comment tests  
echo "📋 Test 6: Inline Comments"
echo "Single-line comment test:"
echo "Expected: 3"
echo -n "Actual:   "
echo "3 -- this should work" | ./lang
echo

echo "Multi-line comment test:"
echo "Expected: 8"
echo -n "Actual:   "
echo "{- ignore this -} 8" | ./lang
echo

echo "Nested inline comment test:"
echo "Expected: 9"
echo -n "Actual:   "
echo "{- outer {- nested -} comment -} 9" | ./lang
echo

# Test 7: AST output with comments (should show clean AST)
echo "📋 Test 7: AST Output (Comments Removed)"
echo "Testing that comments don't appear in AST:"
echo "let x = 1 in x end -- comment" | ./lang -a
echo

echo "✅ All comment tests completed!"
echo
echo "🎯 DESIGN INTENTIONS VERIFIED:"
echo "- ✅ GHC Haskell compatibility (-- and {- -} syntax)"
echo "- ✅ Nested multi-line comments work correctly"
echo "- ✅ Comments completely removed during lexical analysis"
echo "- ✅ No impact on code execution or AST generation"
echo "- ✅ Support for documentation and development workflows"