#!/usr/bin/env bash

set -euo pipefail

EXPECTED_OUTPUT="File content: hello"
EXPECTED_FILE_CONTENT="hello"

fail() {
    echo "[FAIL] $1"
    exit 1
}

assert_eq() {
    local actual="$1"
    local expected="$2"
    local msg="$3"
    if [[ "$actual" != "$expected" ]]; then
        echo "[FAIL] $msg"
        echo "Expected: $expected"
        echo "Actual:   $actual"
        exit 1
    fi
    echo "[PASS] $msg"
}

cleanup_artifacts() {
    rm -f "file_write_read" "sample.txt" "stderr.log"
    rm -rf "tmp_test_dir"
}

trap cleanup_artifacts EXIT

echo "Running test suite for file_write_read.c"

# Test 1: Compilation should succeed.
gcc "file_write_read.c" -o "file_write_read"
echo "[PASS] Compile succeeded"

# Test 2: Fresh run should create file and print expected output.
rm -f "sample.txt"
program_output="$(./file_write_read)"
assert_eq "$program_output" "$EXPECTED_OUTPUT" "Fresh run output matches"
[[ -f "sample.txt" ]] || fail "Fresh run should create sample.txt"
file_content="$(<sample.txt)"
assert_eq "$file_content" "$EXPECTED_FILE_CONTENT" "Fresh run file content matches"

# Test 3: Existing file with different content should be overwritten ("w" mode).
printf "old-content-should-be-replaced" > "sample.txt"
program_output="$(./file_write_read)"
assert_eq "$program_output" "$EXPECTED_OUTPUT" "Overwrite run output matches"
file_content="$(<sample.txt)"
assert_eq "$file_content" "$EXPECTED_FILE_CONTENT" "Overwrite truncates and rewrites content"

# Test 4: Repeated executions should stay consistent.
for i in 1 2 3; do
    program_output="$(./file_write_read)"
    assert_eq "$program_output" "$EXPECTED_OUTPUT" "Repeat run #$i output matches"
    file_content="$(<sample.txt)"
    assert_eq "$file_content" "$EXPECTED_FILE_CONTENT" "Repeat run #$i file content matches"
done

# Test 5: Negative scenario - run in non-writable directory should fail gracefully.
mkdir -p "tmp_test_dir"
cp "file_write_read" "tmp_test_dir/"
chmod 555 "tmp_test_dir"
if (
    cd "tmp_test_dir" && ./file_write_read > /dev/null 2> ../stderr.log
); then
    chmod 755 "tmp_test_dir"
    fail "Program should fail in non-writable directory"
fi
chmod 755 "tmp_test_dir"
[[ -s "stderr.log" ]] || fail "Failure case should produce an error message"
echo "[PASS] Non-writable directory failure scenario handled"

echo "All scenarios passed."
