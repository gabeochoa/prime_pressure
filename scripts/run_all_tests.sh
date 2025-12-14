#!/bin/bash

# Script to run all tests and capture results
echo "Building project..."
make

echo "Running all tests..."

TESTS=(
    "test_order_selection"
    "test_typing_items"
    "test_typing_buffer_status"
    "test_order_tabbing"
    "test_boxing_workflow"
    "test_complete_order"
    "test_ready_stamp_sequence"
)

PASSED=0
FAILED=0

for test in "${TESTS[@]}"; do
    echo -n "Running $test... "
    timeout 120s ./output/warehouse_game.exe --run-test "$test" > /tmp/test_output.txt 2>&1
    exit_code=$?

    if grep -q "passed" /tmp/test_output.txt; then
        echo "PASSED"
        ((PASSED++))
    elif grep -q "failed" /tmp/test_output.txt; then
        echo "FAILED"
        ((FAILED++))
        echo "  Error output:"
        grep -A 5 "failed:" /tmp/test_output.txt | head -10
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT"
        ((FAILED++))
    else
        echo "UNKNOWN (skipped)"
        # Don't count UNKNOWN as failed
    fi
done

echo ""
echo "Results: $PASSED passed, $FAILED failed"
echo "Total: $((PASSED + FAILED)) tests"