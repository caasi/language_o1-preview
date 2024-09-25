#!/bin/bash

# Set the path to your language executable
LANG_EXEC=./lang

# Initialize counters
total_tests=0
passed_tests=0

# Loop over all test input files
for test_file in tests/*.lang; do
    ((total_tests++))
    test_name=$(basename "$test_file" .lang)
    expected_output="tests/$test_name.out"
    actual_output="tests/$test_name.actual"

    # Run the test
    $LANG_EXEC < "$test_file" > "$actual_output" 2>&1

    # Compare the actual output to the expected output
    if diff -q "$expected_output" "$actual_output" > /dev/null; then
        echo "Test $test_name: PASS"
        ((passed_tests++))
        # Remove the actual output file if the test passes
        rm "$actual_output"
    else
        echo "Test $test_name: FAIL"
        echo "Expected Output:"
        cat "$expected_output"
        echo "Actual Output:"
        cat "$actual_output"
        echo "Difference:"
        diff "$expected_output" "$actual_output"
    fi
done

# Summary
echo ""
echo "Total Tests: $total_tests"
echo "Passed Tests: $passed_tests"
echo "Failed Tests: $((total_tests - passed_tests))"

# Exit with non-zero status if any tests failed
if [ $passed_tests -ne $total_tests ]; then
    exit 1
fi
