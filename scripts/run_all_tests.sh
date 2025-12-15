#!/bin/bash

# Script to run all tests and capture results
echo "Building project..."
make

echo "Running all tests..."

TESTS=(
    "test_order_selection"
    "test_typing_items"
    "test_boxing_workflow"
    "test_complete_order"
    "test_ready_stamp_sequence"
)

PASSED=0
FAILED=0

for test in "${TESTS[@]}"; do
    echo -n "Running $test... "
    BIN="./output/warehouse_game"
    if [ ! -f "$BIN" ]; then
        BIN="./output/warehouse_game.exe"
    fi
    RUN_CMD=( "$BIN" --run-test "$test" )
    if command -v xvfb-run >/dev/null 2>&1; then
        RUN_CMD=( xvfb-run -a "${RUN_CMD[@]}" )
    fi
    timeout 120s "${RUN_CMD[@]}" > /tmp/test_output.txt 2>&1
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo "PASSED"
        ((PASSED++))
    elif [ $exit_code -eq 124 ]; then
        echo "TIMEOUT"
        ((FAILED++))
        echo "  Error output:"
        tail -50 /tmp/test_output.txt
    else
        echo "FAILED"
        ((FAILED++))
        echo "  Error output:"
        tail -50 /tmp/test_output.txt
    fi
done

echo ""
echo "Results: $PASSED passed, $FAILED failed"
echo "Total: $((PASSED + FAILED)) tests"